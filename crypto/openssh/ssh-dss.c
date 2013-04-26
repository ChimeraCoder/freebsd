
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"

#include <sys/types.h>

#include <openssl/bn.h>
#include <openssl/evp.h>

#include <stdarg.h>
#include <string.h>

#include "xmalloc.h"
#include "buffer.h"
#include "compat.h"
#include "log.h"
#include "key.h"

#define INTBLOB_LEN	20
#define SIGBLOB_LEN	(2*INTBLOB_LEN)

int
ssh_dss_sign(const Key *key, u_char **sigp, u_int *lenp,
    const u_char *data, u_int datalen)
{
	DSA_SIG *sig;
	const EVP_MD *evp_md = EVP_sha1();
	EVP_MD_CTX md;
	u_char digest[EVP_MAX_MD_SIZE], sigblob[SIGBLOB_LEN];
	u_int rlen, slen, len, dlen;
	Buffer b;

	if (key == NULL || key->dsa == NULL || (key->type != KEY_DSA &&
	    key->type != KEY_DSA_CERT && key->type != KEY_DSA_CERT_V00)) {
		error("ssh_dss_sign: no DSA key");
		return -1;
	}
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, data, datalen);
	EVP_DigestFinal(&md, digest, &dlen);

	sig = DSA_do_sign(digest, dlen, key->dsa);
	memset(digest, 'd', sizeof(digest));

	if (sig == NULL) {
		error("ssh_dss_sign: sign failed");
		return -1;
	}

	rlen = BN_num_bytes(sig->r);
	slen = BN_num_bytes(sig->s);
	if (rlen > INTBLOB_LEN || slen > INTBLOB_LEN) {
		error("bad sig size %u %u", rlen, slen);
		DSA_SIG_free(sig);
		return -1;
	}
	memset(sigblob, 0, SIGBLOB_LEN);
	BN_bn2bin(sig->r, sigblob+ SIGBLOB_LEN - INTBLOB_LEN - rlen);
	BN_bn2bin(sig->s, sigblob+ SIGBLOB_LEN - slen);
	DSA_SIG_free(sig);

	if (datafellows & SSH_BUG_SIGBLOB) {
		if (lenp != NULL)
			*lenp = SIGBLOB_LEN;
		if (sigp != NULL) {
			*sigp = xmalloc(SIGBLOB_LEN);
			memcpy(*sigp, sigblob, SIGBLOB_LEN);
		}
	} else {
		/* ietf-drafts */
		buffer_init(&b);
		buffer_put_cstring(&b, "ssh-dss");
		buffer_put_string(&b, sigblob, SIGBLOB_LEN);
		len = buffer_len(&b);
		if (lenp != NULL)
			*lenp = len;
		if (sigp != NULL) {
			*sigp = xmalloc(len);
			memcpy(*sigp, buffer_ptr(&b), len);
		}
		buffer_free(&b);
	}
	return 0;
}
int
ssh_dss_verify(const Key *key, const u_char *signature, u_int signaturelen,
    const u_char *data, u_int datalen)
{
	DSA_SIG *sig;
	const EVP_MD *evp_md = EVP_sha1();
	EVP_MD_CTX md;
	u_char digest[EVP_MAX_MD_SIZE], *sigblob;
	u_int len, dlen;
	int rlen, ret;
	Buffer b;

	if (key == NULL || key->dsa == NULL || (key->type != KEY_DSA &&
	    key->type != KEY_DSA_CERT && key->type != KEY_DSA_CERT_V00)) {
		error("ssh_dss_verify: no DSA key");
		return -1;
	}

	/* fetch signature */
	if (datafellows & SSH_BUG_SIGBLOB) {
		sigblob = xmalloc(signaturelen);
		memcpy(sigblob, signature, signaturelen);
		len = signaturelen;
	} else {
		/* ietf-drafts */
		char *ktype;
		buffer_init(&b);
		buffer_append(&b, signature, signaturelen);
		ktype = buffer_get_cstring(&b, NULL);
		if (strcmp("ssh-dss", ktype) != 0) {
			error("ssh_dss_verify: cannot handle type %s", ktype);
			buffer_free(&b);
			xfree(ktype);
			return -1;
		}
		xfree(ktype);
		sigblob = buffer_get_string(&b, &len);
		rlen = buffer_len(&b);
		buffer_free(&b);
		if (rlen != 0) {
			error("ssh_dss_verify: "
			    "remaining bytes in signature %d", rlen);
			xfree(sigblob);
			return -1;
		}
	}

	if (len != SIGBLOB_LEN) {
		fatal("bad sigbloblen %u != SIGBLOB_LEN", len);
	}

	/* parse signature */
	if ((sig = DSA_SIG_new()) == NULL)
		fatal("ssh_dss_verify: DSA_SIG_new failed");
	if ((sig->r = BN_new()) == NULL)
		fatal("ssh_dss_verify: BN_new failed");
	if ((sig->s = BN_new()) == NULL)
		fatal("ssh_dss_verify: BN_new failed");
	if ((BN_bin2bn(sigblob, INTBLOB_LEN, sig->r) == NULL) ||
	    (BN_bin2bn(sigblob+ INTBLOB_LEN, INTBLOB_LEN, sig->s) == NULL))
		fatal("ssh_dss_verify: BN_bin2bn failed");

	/* clean up */
	memset(sigblob, 0, len);
	xfree(sigblob);

	/* sha1 the data */
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, data, datalen);
	EVP_DigestFinal(&md, digest, &dlen);

	ret = DSA_do_verify(digest, dlen, sig, key->dsa);
	memset(digest, 'd', sizeof(digest));

	DSA_SIG_free(sig);

	debug("ssh_dss_verify: signature %s",
	    ret == 1 ? "correct" : ret == 0 ? "incorrect" : "error");
	return ret;
}
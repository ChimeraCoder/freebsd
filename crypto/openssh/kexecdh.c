
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

#ifdef OPENSSL_HAS_ECC

#include <sys/types.h>

#include <signal.h>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>

#include "buffer.h"
#include "ssh2.h"
#include "key.h"
#include "cipher.h"
#include "kex.h"
#include "log.h"

int
kex_ecdh_name_to_nid(const char *kexname)
{
	if (strlen(kexname) < sizeof(KEX_ECDH_SHA2_STEM) - 1)
		fatal("%s: kexname too short \"%s\"", __func__, kexname);
	return key_curve_name_to_nid(kexname + sizeof(KEX_ECDH_SHA2_STEM) - 1);
}

const EVP_MD *
kex_ecdh_name_to_evpmd(const char *kexname)
{
	int nid = kex_ecdh_name_to_nid(kexname);

	if (nid == -1)
		fatal("%s: unsupported ECDH curve \"%s\"", __func__, kexname);
	return key_ec_nid_to_evpmd(nid);
}

void
kex_ecdh_hash(
    const EVP_MD *evp_md,
    const EC_GROUP *ec_group,
    char *client_version_string,
    char *server_version_string,
    char *ckexinit, int ckexinitlen,
    char *skexinit, int skexinitlen,
    u_char *serverhostkeyblob, int sbloblen,
    const EC_POINT *client_dh_pub,
    const EC_POINT *server_dh_pub,
    const BIGNUM *shared_secret,
    u_char **hash, u_int *hashlen)
{
	Buffer b;
	EVP_MD_CTX md;
	static u_char digest[EVP_MAX_MD_SIZE];

	buffer_init(&b);
	buffer_put_cstring(&b, client_version_string);
	buffer_put_cstring(&b, server_version_string);

	/* kexinit messages: fake header: len+SSH2_MSG_KEXINIT */
	buffer_put_int(&b, ckexinitlen+1);
	buffer_put_char(&b, SSH2_MSG_KEXINIT);
	buffer_append(&b, ckexinit, ckexinitlen);
	buffer_put_int(&b, skexinitlen+1);
	buffer_put_char(&b, SSH2_MSG_KEXINIT);
	buffer_append(&b, skexinit, skexinitlen);

	buffer_put_string(&b, serverhostkeyblob, sbloblen);
	buffer_put_ecpoint(&b, ec_group, client_dh_pub);
	buffer_put_ecpoint(&b, ec_group, server_dh_pub);
	buffer_put_bignum2(&b, shared_secret);

#ifdef DEBUG_KEX
	buffer_dump(&b);
#endif
	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, buffer_ptr(&b), buffer_len(&b));
	EVP_DigestFinal(&md, digest, NULL);

	buffer_free(&b);

#ifdef DEBUG_KEX
	dump_digest("hash", digest, EVP_MD_size(evp_md));
#endif
	*hash = digest;
	*hashlen = EVP_MD_size(evp_md);
}

#endif /* OPENSSL_HAS_ECC */

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

#include <openssl/evp.h>
#include <signal.h>

#include "buffer.h"
#include "key.h"
#include "cipher.h"
#include "kex.h"
#include "ssh2.h"

void
kexgex_hash(
    const EVP_MD *evp_md,
    char *client_version_string,
    char *server_version_string,
    char *ckexinit, int ckexinitlen,
    char *skexinit, int skexinitlen,
    u_char *serverhostkeyblob, int sbloblen,
    int min, int wantbits, int max, BIGNUM *prime, BIGNUM *gen,
    BIGNUM *client_dh_pub,
    BIGNUM *server_dh_pub,
    BIGNUM *shared_secret,
    u_char **hash, u_int *hashlen)
{
	Buffer b;
	static u_char digest[EVP_MAX_MD_SIZE];
	EVP_MD_CTX md;

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
	if (min == -1 || max == -1)
		buffer_put_int(&b, wantbits);
	else {
		buffer_put_int(&b, min);
		buffer_put_int(&b, wantbits);
		buffer_put_int(&b, max);
	}
	buffer_put_bignum2(&b, prime);
	buffer_put_bignum2(&b, gen);
	buffer_put_bignum2(&b, client_dh_pub);
	buffer_put_bignum2(&b, server_dh_pub);
	buffer_put_bignum2(&b, shared_secret);

#ifdef DEBUG_KEXDH
	buffer_dump(&b);
#endif

	EVP_DigestInit(&md, evp_md);
	EVP_DigestUpdate(&md, buffer_ptr(&b), buffer_len(&b));
	EVP_DigestFinal(&md, digest, NULL);

	buffer_free(&b);
	*hash = digest;
	*hashlen = EVP_MD_size(evp_md);
#ifdef DEBUG_KEXDH
	dump_digest("hash", digest, *hashlen);
#endif
}
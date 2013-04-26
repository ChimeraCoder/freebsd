
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

#include <openssl/opensslconf.h>
#ifndef OPENSSL_NO_CAMELLIA
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <assert.h>
#include <openssl/camellia.h>
#include "evp_locl.h"

static int camellia_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
	const unsigned char *iv, int enc);

/* Camellia subkey Structure */
typedef struct
	{
	CAMELLIA_KEY ks;
	} EVP_CAMELLIA_KEY;

/* Attribute operation for Camellia */
#define data(ctx)	EVP_C_DATA(EVP_CAMELLIA_KEY,ctx)

IMPLEMENT_BLOCK_CIPHER(camellia_128, ks, Camellia, EVP_CAMELLIA_KEY,
	NID_camellia_128, 16, 16, 16, 128,
	0, camellia_init_key, NULL, 
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL)
IMPLEMENT_BLOCK_CIPHER(camellia_192, ks, Camellia, EVP_CAMELLIA_KEY,
	NID_camellia_192, 16, 24, 16, 128,
	0, camellia_init_key, NULL, 
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL)
IMPLEMENT_BLOCK_CIPHER(camellia_256, ks, Camellia, EVP_CAMELLIA_KEY,
	NID_camellia_256, 16, 32, 16, 128,
	0, camellia_init_key, NULL, 
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL)

#define IMPLEMENT_CAMELLIA_CFBR(ksize,cbits)	IMPLEMENT_CFBR(camellia,Camellia,EVP_CAMELLIA_KEY,ks,ksize,cbits,16)

IMPLEMENT_CAMELLIA_CFBR(128,1)
IMPLEMENT_CAMELLIA_CFBR(192,1)
IMPLEMENT_CAMELLIA_CFBR(256,1)

IMPLEMENT_CAMELLIA_CFBR(128,8)
IMPLEMENT_CAMELLIA_CFBR(192,8)
IMPLEMENT_CAMELLIA_CFBR(256,8)



/* The subkey for Camellia is generated. */ 
static int camellia_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
	const unsigned char *iv, int enc)
	{
	int ret;

	ret=Camellia_set_key(key, ctx->key_len * 8, ctx->cipher_data);

	if(ret < 0)
		{
		EVPerr(EVP_F_CAMELLIA_INIT_KEY,EVP_R_CAMELLIA_KEY_SETUP_FAILED);
		return 0;
		}

	return 1;
	}

#else

# ifdef PEDANTIC
static void *dummy=&dummy;
# endif

#endif
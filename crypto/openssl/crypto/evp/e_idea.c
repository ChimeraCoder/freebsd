
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

#include <stdio.h>
#include "cryptlib.h"

#ifndef OPENSSL_NO_IDEA
#include <openssl/evp.h>
#include <openssl/objects.h>
#include "evp_locl.h"
#include <openssl/idea.h>

static int idea_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			 const unsigned char *iv,int enc);

/* NB idea_ecb_encrypt doesn't take an 'encrypt' argument so we treat it as a special
 * case 
 */

static int idea_ecb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
			   const unsigned char *in, size_t inl)
{
	BLOCK_CIPHER_ecb_loop()
		idea_ecb_encrypt(in + i, out + i, ctx->cipher_data);
	return 1;
}

/* Can't use IMPLEMENT_BLOCK_CIPHER because idea_ecb_encrypt is different */

typedef struct
	{
	IDEA_KEY_SCHEDULE ks;
	} EVP_IDEA_KEY;

BLOCK_CIPHER_func_cbc(idea, idea, EVP_IDEA_KEY, ks)
BLOCK_CIPHER_func_ofb(idea, idea, 64, EVP_IDEA_KEY, ks)
BLOCK_CIPHER_func_cfb(idea, idea, 64, EVP_IDEA_KEY, ks)

BLOCK_CIPHER_defs(idea, IDEA_KEY_SCHEDULE, NID_idea, 8, 16, 8, 64,
			0, idea_init_key, NULL, 
			EVP_CIPHER_set_asn1_iv, EVP_CIPHER_get_asn1_iv, NULL)

static int idea_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			 const unsigned char *iv, int enc)
	{
	if(!enc) {
		if (EVP_CIPHER_CTX_mode(ctx) == EVP_CIPH_OFB_MODE) enc = 1;
		else if (EVP_CIPHER_CTX_mode(ctx) == EVP_CIPH_CFB_MODE) enc = 1;
	}
	if (enc) idea_set_encrypt_key(key,ctx->cipher_data);
	else
		{
		IDEA_KEY_SCHEDULE tmp;

		idea_set_encrypt_key(key,&tmp);
		idea_set_decrypt_key(&tmp,ctx->cipher_data);
		OPENSSL_cleanse((unsigned char *)&tmp,
				sizeof(IDEA_KEY_SCHEDULE));
		}
	return 1;
	}

#endif
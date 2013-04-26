
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

#ifndef OPENSSL_NO_CAST
#include <openssl/evp.h>
#include <openssl/objects.h>
#include "evp_locl.h"
#include <openssl/cast.h>

static int cast_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			 const unsigned char *iv,int enc);

typedef struct
	{
	CAST_KEY ks;
	} EVP_CAST_KEY;

#define data(ctx)	EVP_C_DATA(EVP_CAST_KEY,ctx)

IMPLEMENT_BLOCK_CIPHER(cast5, ks, CAST, EVP_CAST_KEY, 
			NID_cast5, 8, CAST_KEY_LENGTH, 8, 64,
			EVP_CIPH_VARIABLE_LENGTH, cast_init_key, NULL,
			EVP_CIPHER_set_asn1_iv, EVP_CIPHER_get_asn1_iv, NULL)
			
static int cast_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			 const unsigned char *iv, int enc)
	{
	CAST_set_key(&data(ctx)->ks,EVP_CIPHER_CTX_key_length(ctx),key);
	return 1;
	}

#endif
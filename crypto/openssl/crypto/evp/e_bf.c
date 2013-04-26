
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
#ifndef OPENSSL_NO_BF
#include <openssl/evp.h>
#include "evp_locl.h"
#include <openssl/objects.h>
#include <openssl/blowfish.h>

static int bf_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
		       const unsigned char *iv, int enc);

typedef struct
	{
	BF_KEY ks;
	} EVP_BF_KEY;

#define data(ctx)	EVP_C_DATA(EVP_BF_KEY,ctx)

IMPLEMENT_BLOCK_CIPHER(bf, ks, BF, EVP_BF_KEY, NID_bf, 8, 16, 8, 64,
			EVP_CIPH_VARIABLE_LENGTH, bf_init_key, NULL, 
			EVP_CIPHER_set_asn1_iv, EVP_CIPHER_get_asn1_iv, NULL)
	
static int bf_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
		       const unsigned char *iv, int enc)
	{
	BF_set_key(&data(ctx)->ks,EVP_CIPHER_CTX_key_length(ctx),key);
	return 1;
	}

#endif

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

#ifndef OPENSSL_NO_DES

#include <openssl/evp.h>
#include <openssl/objects.h>
#include "evp_locl.h"
#include <openssl/des.h>

static int desx_cbc_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			     const unsigned char *iv,int enc);
static int desx_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
			   const unsigned char *in, size_t inl);


typedef struct
    {
    DES_key_schedule ks;/* key schedule */
    DES_cblock inw;
    DES_cblock outw;
    } DESX_CBC_KEY;

#define data(ctx) ((DESX_CBC_KEY *)(ctx)->cipher_data)

static const EVP_CIPHER d_xcbc_cipher=
	{
	NID_desx_cbc,
	8,24,8,
	EVP_CIPH_CBC_MODE,
	desx_cbc_init_key,
	desx_cbc_cipher,
	NULL,
	sizeof(DESX_CBC_KEY),
	EVP_CIPHER_set_asn1_iv,
	EVP_CIPHER_get_asn1_iv,
	NULL,
	NULL
	};

const EVP_CIPHER *EVP_desx_cbc(void)
	{
	return(&d_xcbc_cipher);
	}
	
static int desx_cbc_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			     const unsigned char *iv, int enc)
	{
	DES_cblock *deskey = (DES_cblock *)key;

	DES_set_key_unchecked(deskey,&data(ctx)->ks);
	memcpy(&data(ctx)->inw[0],&key[8],8);
	memcpy(&data(ctx)->outw[0],&key[16],8);

	return 1;
	}

static int desx_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
			   const unsigned char *in, size_t inl)
	{
	while (inl>=EVP_MAXCHUNK)
		{
		DES_xcbc_encrypt(in,out,(long)EVP_MAXCHUNK,&data(ctx)->ks,
			 (DES_cblock *)&(ctx->iv[0]),
			 &data(ctx)->inw,
			 &data(ctx)->outw,
			 ctx->encrypt);
		inl-=EVP_MAXCHUNK;
		in +=EVP_MAXCHUNK;
		out+=EVP_MAXCHUNK;
		}
	if (inl)
		DES_xcbc_encrypt(in,out,(long)inl,&data(ctx)->ks,
			(DES_cblock *)&(ctx->iv[0]),
			&data(ctx)->inw,
			&data(ctx)->outw,
			ctx->encrypt);
	return 1;
	}
#endif
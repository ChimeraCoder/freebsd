
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

#ifndef OPENSSL_NO_RC4

#include <openssl/evp.h>
#include "evp_locl.h"
#include <openssl/objects.h>
#include <openssl/rc4.h>

/* FIXME: surely this is available elsewhere? */
#define EVP_RC4_KEY_SIZE		16

typedef struct
    {
    RC4_KEY ks;	/* working key */
    } EVP_RC4_KEY;

#define data(ctx) ((EVP_RC4_KEY *)(ctx)->cipher_data)

static int rc4_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			const unsigned char *iv,int enc);
static int rc4_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
		      const unsigned char *in, size_t inl);
static const EVP_CIPHER r4_cipher=
	{
	NID_rc4,
	1,EVP_RC4_KEY_SIZE,0,
	EVP_CIPH_VARIABLE_LENGTH,
	rc4_init_key,
	rc4_cipher,
	NULL,
	sizeof(EVP_RC4_KEY),
	NULL,
	NULL,
	NULL,
	NULL
	};

static const EVP_CIPHER r4_40_cipher=
	{
	NID_rc4_40,
	1,5 /* 40 bit */,0,
	EVP_CIPH_VARIABLE_LENGTH,
	rc4_init_key,
	rc4_cipher,
	NULL,
	sizeof(EVP_RC4_KEY),
	NULL, 
	NULL,
	NULL,
	NULL
	};

const EVP_CIPHER *EVP_rc4(void)
	{
	return(&r4_cipher);
	}

const EVP_CIPHER *EVP_rc4_40(void)
	{
	return(&r4_40_cipher);
	}

static int rc4_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			const unsigned char *iv, int enc)
	{
	RC4_set_key(&data(ctx)->ks,EVP_CIPHER_CTX_key_length(ctx),
		    key);
	return 1;
	}

static int rc4_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out,
		      const unsigned char *in, size_t inl)
	{
	RC4(&data(ctx)->ks,inl,in,out);
	return 1;
	}
#endif
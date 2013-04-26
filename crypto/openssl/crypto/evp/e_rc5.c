
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

#ifndef OPENSSL_NO_RC5

#include <openssl/evp.h>
#include <openssl/objects.h>
#include "evp_locl.h"
#include <openssl/rc5.h>

static int r_32_12_16_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			       const unsigned char *iv,int enc);
static int rc5_ctrl(EVP_CIPHER_CTX *c, int type, int arg, void *ptr);

typedef struct
	{
	int rounds;	/* number of rounds */
	RC5_32_KEY ks;	/* key schedule */
	} EVP_RC5_KEY;

#define data(ctx)	EVP_C_DATA(EVP_RC5_KEY,ctx)

IMPLEMENT_BLOCK_CIPHER(rc5_32_12_16, ks, RC5_32, EVP_RC5_KEY, NID_rc5,
		       8, RC5_32_KEY_LENGTH, 8, 64,
		       EVP_CIPH_VARIABLE_LENGTH | EVP_CIPH_CTRL_INIT,
		       r_32_12_16_init_key, NULL,
		       NULL, NULL, rc5_ctrl)

static int rc5_ctrl(EVP_CIPHER_CTX *c, int type, int arg, void *ptr)
	{
	switch(type)
		{
	case EVP_CTRL_INIT:
		data(c)->rounds = RC5_12_ROUNDS;
		return 1;

	case EVP_CTRL_GET_RC5_ROUNDS:
		*(int *)ptr = data(c)->rounds;
		return 1;
			
	case EVP_CTRL_SET_RC5_ROUNDS:
		switch(arg)
			{
		case RC5_8_ROUNDS:
		case RC5_12_ROUNDS:
		case RC5_16_ROUNDS:
			data(c)->rounds = arg;
			return 1;

		default:
			EVPerr(EVP_F_RC5_CTRL, EVP_R_UNSUPORTED_NUMBER_OF_ROUNDS);
			return 0;
			}

	default:
		return -1;
		}
	}

static int r_32_12_16_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
			       const unsigned char *iv, int enc)
	{
	RC5_32_set_key(&data(ctx)->ks,EVP_CIPHER_CTX_key_length(ctx),
		       key,data(ctx)->rounds);
	return 1;
	}

#endif

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
#ifndef OPENSSL_NO_SEED
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <assert.h>
#include <openssl/seed.h>
#include "evp_locl.h"

static int seed_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,	const unsigned char *iv, int enc);

typedef struct
	{
	SEED_KEY_SCHEDULE ks;
	} EVP_SEED_KEY;

IMPLEMENT_BLOCK_CIPHER(seed, ks, SEED, EVP_SEED_KEY, NID_seed,
                       16, 16, 16, 128,
                       0, seed_init_key, 0, 0, 0, 0)

static int seed_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
                         const unsigned char *iv, int enc)
	{
	SEED_set_key(key, ctx->cipher_data);
	return 1;
	}

#endif
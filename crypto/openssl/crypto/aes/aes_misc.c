
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

#include <openssl/opensslv.h>
#include <openssl/crypto.h>
#include <openssl/aes.h>
#include "aes_locl.h"

const char AES_version[]="AES" OPENSSL_VERSION_PTEXT;

const char *AES_options(void) {
#ifdef FULL_UNROLL
        return "aes(full)";
#else   
        return "aes(partial)";
#endif
}

/* FIPS wrapper functions to block low level AES calls in FIPS mode */

int AES_set_encrypt_key(const unsigned char *userKey, const int bits,
			AES_KEY *key)
	{
#ifdef OPENSSL_FIPS
	fips_cipher_abort(AES);
#endif
	return private_AES_set_encrypt_key(userKey, bits, key);
	}

int AES_set_decrypt_key(const unsigned char *userKey, const int bits,
			AES_KEY *key)
	{
#ifdef OPENSSL_FIPS
	fips_cipher_abort(AES);
#endif
	return private_AES_set_decrypt_key(userKey, bits, key);
	}
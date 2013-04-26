
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

#ifndef AES_DEBUG
# ifndef NDEBUG
#  define NDEBUG
# endif
#endif
#include <assert.h>

#include <openssl/aes.h>
#include "aes_locl.h"

void AES_ecb_encrypt(const unsigned char *in, unsigned char *out,
		     const AES_KEY *key, const int enc) {

        assert(in && out && key);
	assert((AES_ENCRYPT == enc)||(AES_DECRYPT == enc));

	if (AES_ENCRYPT == enc)
		AES_encrypt(in, out, key);
	else
		AES_decrypt(in, out, key);
}

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

#include <openssl/aes.h>
#include <openssl/modes.h>

void AES_ctr128_encrypt(const unsigned char *in, unsigned char *out,
			size_t length, const AES_KEY *key,
			unsigned char ivec[AES_BLOCK_SIZE],
			unsigned char ecount_buf[AES_BLOCK_SIZE],
			unsigned int *num) {
	CRYPTO_ctr128_encrypt(in,out,length,key,ivec,ecount_buf,num,(block128_f)AES_encrypt);
}
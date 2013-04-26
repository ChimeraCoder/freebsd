
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

#include <openssl/camellia.h>
#include <openssl/modes.h>

/* The input and output encrypted as though 128bit ofb mode is being
 * used.  The extra state information to record how much of the
 * 128bit block we have used is contained in *num;
 */
void Camellia_ofb128_encrypt(const unsigned char *in, unsigned char *out,
	size_t length, const CAMELLIA_KEY *key,
	unsigned char *ivec, int *num) {
	CRYPTO_ofb128_encrypt(in,out,length,key,ivec,num,(block128_f)Camellia_encrypt);
}
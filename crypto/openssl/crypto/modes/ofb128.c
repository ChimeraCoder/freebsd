
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

#include <openssl/crypto.h>
#include "modes_lcl.h"
#include <string.h>

#ifndef MODES_DEBUG
# ifndef NDEBUG
#  define NDEBUG
# endif
#endif
#include <assert.h>

/* The input and output encrypted as though 128bit ofb mode is being
 * used.  The extra state information to record how much of the
 * 128bit block we have used is contained in *num;
 */
void CRYPTO_ofb128_encrypt(const unsigned char *in, unsigned char *out,
			size_t len, const void *key,
			unsigned char ivec[16], int *num,
			block128_f block)
{
	unsigned int n;
	size_t l=0;

	assert(in && out && key && ivec && num);

	n = *num;

#if !defined(OPENSSL_SMALL_FOOTPRINT)
	if (16%sizeof(size_t) == 0) do { /* always true actually */
		while (n && len) {
			*(out++) = *(in++) ^ ivec[n];
			--len;
			n = (n+1) % 16;
		}
#if defined(STRICT_ALIGNMENT)
		if (((size_t)in|(size_t)out|(size_t)ivec)%sizeof(size_t) != 0)
			break;
#endif
		while (len>=16) {
			(*block)(ivec, ivec, key);
			for (; n<16; n+=sizeof(size_t))
				*(size_t*)(out+n) =
				*(size_t*)(in+n) ^ *(size_t*)(ivec+n);
			len -= 16;
			out += 16;
			in  += 16;
			n = 0;
		}
		if (len) {
			(*block)(ivec, ivec, key);
			while (len--) {
				out[n] = in[n] ^ ivec[n];
				++n;
			}
		}
		*num = n;
		return;
	} while(0);
	/* the rest would be commonly eliminated by x86* compiler */
#endif
	while (l<len) {
		if (n==0) {
			(*block)(ivec, ivec, key);
		}
		out[l] = in[l] ^ ivec[n];
		++l;
		n = (n+1) % 16;
	}

	*num=n;
}

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

#ifndef STRICT_ALIGNMENT
#  define STRICT_ALIGNMENT 0
#endif

void CRYPTO_cbc128_encrypt(const unsigned char *in, unsigned char *out,
			size_t len, const void *key,
			unsigned char ivec[16], block128_f block)
{
	size_t n;
	const unsigned char *iv = ivec;

	assert(in && out && key && ivec);

#if !defined(OPENSSL_SMALL_FOOTPRINT)
	if (STRICT_ALIGNMENT &&
	    ((size_t)in|(size_t)out|(size_t)ivec)%sizeof(size_t) != 0) {
		while (len>=16) {
			for(n=0; n<16; ++n)
				out[n] = in[n] ^ iv[n];
			(*block)(out, out, key);
			iv = out;
			len -= 16;
			in  += 16;
			out += 16;
		}
	} else {
		while (len>=16) {
			for(n=0; n<16; n+=sizeof(size_t))
				*(size_t*)(out+n) =
				*(size_t*)(in+n) ^ *(size_t*)(iv+n);
			(*block)(out, out, key);
			iv = out;
			len -= 16;
			in  += 16;
			out += 16;
		}
	}
#endif
	while (len) {
		for(n=0; n<16 && n<len; ++n)
			out[n] = in[n] ^ iv[n];
		for(; n<16; ++n)
			out[n] = iv[n];
		(*block)(out, out, key);
		iv = out;
		if (len<=16) break;
		len -= 16;
		in  += 16;
		out += 16;
	}
	memcpy(ivec,iv,16);
}

void CRYPTO_cbc128_decrypt(const unsigned char *in, unsigned char *out,
			size_t len, const void *key,
			unsigned char ivec[16], block128_f block)
{
	size_t n;
	union { size_t align; unsigned char c[16]; } tmp;

	assert(in && out && key && ivec);

#if !defined(OPENSSL_SMALL_FOOTPRINT)
	if (in != out) {
		const unsigned char *iv = ivec;

		if (STRICT_ALIGNMENT &&
		    ((size_t)in|(size_t)out|(size_t)ivec)%sizeof(size_t) != 0) {
			while (len>=16) {
				(*block)(in, out, key);
				for(n=0; n<16; ++n)
					out[n] ^= iv[n];
				iv = in;
				len -= 16;
				in  += 16;
				out += 16;
			}
		}
		else {
			while (len>=16) {
				(*block)(in, out, key);
				for(n=0; n<16; n+=sizeof(size_t))
					*(size_t *)(out+n) ^= *(size_t *)(iv+n);
				iv = in;
				len -= 16;
				in  += 16;
				out += 16;
			}
		}
		memcpy(ivec,iv,16);
	} else {
		if (STRICT_ALIGNMENT &&
		    ((size_t)in|(size_t)out|(size_t)ivec)%sizeof(size_t) != 0) {
			unsigned char c;
			while (len>=16) {
				(*block)(in, tmp.c, key);
				for(n=0; n<16; ++n) {
					c = in[n];
					out[n] = tmp.c[n] ^ ivec[n];
					ivec[n] = c;
				}
				len -= 16;
				in  += 16;
				out += 16;
			}
		}
		else {
			size_t c;
			while (len>=16) {
				(*block)(in, tmp.c, key);
				for(n=0; n<16; n+=sizeof(size_t)) {
					c = *(size_t *)(in+n);
					*(size_t *)(out+n) =
					*(size_t *)(tmp.c+n) ^ *(size_t *)(ivec+n);
					*(size_t *)(ivec+n) = c;
				}
				len -= 16;
				in  += 16;
				out += 16;
			}
		}
	}
#endif
	while (len) {
		unsigned char c;
		(*block)(in, tmp.c, key);
		for(n=0; n<16 && n<len; ++n) {
			c = in[n];
			out[n] = tmp.c[n] ^ ivec[n];
			ivec[n] = c;
		}
		if (len<=16) {
			for (; n<16; ++n)
				ivec[n] = in[n];
			break;
		}
		len -= 16;
		in  += 16;
		out += 16;
	}
}

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

/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Zero-length encoding.  This is a fast and simple algorithm to eliminate
 * runs of zeroes.  Each chunk of compressed data begins with a length byte, b.
 * If b < n (where n is the compression parameter) then the next b + 1 bytes
 * are literal values.  If b >= n then the next (256 - b + 1) bytes are zero.
 */
#include <sys/types.h>
#include <sys/sysmacros.h>

size_t
zle_compress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	uchar_t *src = s_start;
	uchar_t *dst = d_start;
	uchar_t *s_end = src + s_len;
	uchar_t *d_end = dst + d_len;

	while (src < s_end && dst < d_end - 1) {
		uchar_t *first = src;
		uchar_t *len = dst++;
		if (src[0] == 0) {
			uchar_t *last = src + (256 - n);
			while (src < MIN(last, s_end) && src[0] == 0)
				src++;
			*len = src - first - 1 + n;
		} else {
			uchar_t *last = src + n;
			if (d_end - dst < n)
				break;
			while (src < MIN(last, s_end) - 1 && (src[0] | src[1]))
				*dst++ = *src++;
			if (src[0])
				*dst++ = *src++;
			*len = src - first - 1;
		}
	}
	return (src == s_end ? dst - (uchar_t *)d_start : s_len);
}

int
zle_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	uchar_t *src = s_start;
	uchar_t *dst = d_start;
	uchar_t *s_end = src + s_len;
	uchar_t *d_end = dst + d_len;

	while (src < s_end && dst < d_end) {
		int len = 1 + *src++;
		if (len <= n) {
			while (len-- != 0)
				*dst++ = *src++;
		} else {
			len -= n;
			while (len-- != 0)
				*dst++ = 0;
		}
	}
	return (dst == d_end ? 0 : -1);
}
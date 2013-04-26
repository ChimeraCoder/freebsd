
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

static int
zle_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	unsigned char *src = s_start;
	unsigned char *dst = d_start;
	unsigned char *s_end = src + s_len;
	unsigned char *d_end = dst + d_len;

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

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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*#pragma ident	"%Z%%M%	%I%	%E% SMI"*/

/*
 * We keep our own copy of this algorithm for 2 main reasons:
 * 	1. If we didn't, anyone modifying common/os/compress.c would
 *         directly break our on disk format
 * 	2. Our version of lzjb does not have a number of checks that the
 *         common/os version needs and uses
 * In particular, we are adding the "feature" that compress() can
 * take a destination buffer size and return -1 if the data will not
 * compress to d_len or less.
 */

#define	MATCH_BITS	6
#define	MATCH_MIN	3
#define	MATCH_MAX	((1 << MATCH_BITS) + (MATCH_MIN - 1))
#define	OFFSET_MASK	((1 << (16 - MATCH_BITS)) - 1)
#define	LEMPEL_SIZE	256

/*ARGSUSED*/
static int
lzjb_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	unsigned char *src = s_start;
	unsigned char *dst = d_start;
	unsigned char *d_end = (unsigned char *)d_start + d_len;
	unsigned char *cpy, copymap = 0;
	int copymask = 1 << (NBBY - 1);

	while (dst < d_end) {
		if ((copymask <<= 1) == (1 << NBBY)) {
			copymask = 1;
			copymap = *src++;
		}
		if (copymap & copymask) {
			int mlen = (src[0] >> (NBBY - MATCH_BITS)) + MATCH_MIN;
			int offset = ((src[0] << NBBY) | src[1]) & OFFSET_MASK;
			src += 2;
			if ((cpy = dst - offset) < (unsigned char *)d_start)
				return (-1);
			while (--mlen >= 0 && dst < d_end)
				*dst++ = *cpy++;
		} else {
			*dst++ = *src++;
		}
	}
	return (0);
}
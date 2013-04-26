
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
 * Copyright (c) 2005, 2010, Oracle and/or its affiliates. All rights reserved.
 */

/*
 * We keep our own copy of this algorithm for 3 main reasons:
 *	1. If we didn't, anyone modifying common/os/compress.c would
 *         directly break our on disk format
 *	2. Our version of lzjb does not have a number of checks that the
 *         common/os version needs and uses
 *	3. We initialize the lempel to ensure deterministic results,
 *	   so that identical blocks can always be deduplicated.
 * In particular, we are adding the "feature" that compress() can
 * take a destination buffer size and returns the compressed length, or the
 * source length if compression would overflow the destination buffer.
 */

#include <sys/zfs_context.h>
#include <sys/types.h>
#include <sys/param.h>

#define	MATCH_BITS	6
#define	MATCH_MIN	3
#define	MATCH_MAX	((1 << MATCH_BITS) + (MATCH_MIN - 1))
#define	OFFSET_MASK	((1 << (16 - MATCH_BITS)) - 1)
#define	LEMPEL_SIZE	1024

/*ARGSUSED*/
size_t
lzjb_compress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	uchar_t *src = s_start;
	uchar_t *dst = d_start;
	uchar_t *cpy;
	uchar_t *copymap = NULL;
	int copymask = 1 << (NBBY - 1);
	int mlen, offset, hash;
	uint16_t *hp;
	uint16_t lempel[LEMPEL_SIZE] = { 0 };

	while (src < (uchar_t *)s_start + s_len) {
		if ((copymask <<= 1) == (1 << NBBY)) {
			if (dst >= (uchar_t *)d_start + d_len - 1 - 2 * NBBY)
				return (s_len);
			copymask = 1;
			copymap = dst;
			*dst++ = 0;
		}
		if (src > (uchar_t *)s_start + s_len - MATCH_MAX) {
			*dst++ = *src++;
			continue;
		}
		hash = (src[0] << 16) + (src[1] << 8) + src[2];
		hash += hash >> 9;
		hash += hash >> 5;
		hp = &lempel[hash & (LEMPEL_SIZE - 1)];
		offset = (intptr_t)(src - *hp) & OFFSET_MASK;
		*hp = (uint16_t)(uintptr_t)src;
		cpy = src - offset;
		if (cpy >= (uchar_t *)s_start && cpy != src &&
		    src[0] == cpy[0] && src[1] == cpy[1] && src[2] == cpy[2]) {
			*copymap |= copymask;
			for (mlen = MATCH_MIN; mlen < MATCH_MAX; mlen++)
				if (src[mlen] != cpy[mlen])
					break;
			*dst++ = ((mlen - MATCH_MIN) << (NBBY - MATCH_BITS)) |
			    (offset >> NBBY);
			*dst++ = (uchar_t)offset;
			src += mlen;
		} else {
			*dst++ = *src++;
		}
	}
	return (dst - (uchar_t *)d_start);
}

/*ARGSUSED*/
int
lzjb_decompress(void *s_start, void *d_start, size_t s_len, size_t d_len, int n)
{
	uchar_t *src = s_start;
	uchar_t *dst = d_start;
	uchar_t *d_end = (uchar_t *)d_start + d_len;
	uchar_t *cpy;
	uchar_t copymap = 0;
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
			if ((cpy = dst - offset) < (uchar_t *)d_start)
				return (-1);
			while (--mlen >= 0 && dst < d_end)
				*dst++ = *cpy++;
		} else {
			*dst++ = *src++;
		}
	}
	return (0);
}
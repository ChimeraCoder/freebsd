
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <limits.h>
#include <runetype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

static size_t	_ascii_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static int	_ascii_mbsinit(const mbstate_t *);
static size_t	_ascii_mbsnrtowcs(wchar_t * __restrict dst,
		    const char ** __restrict src, size_t nms, size_t len,
		    mbstate_t * __restrict ps __unused);
static size_t	_ascii_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);
static size_t	_ascii_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict);

int
_ascii_init(struct xlocale_ctype *l,_RuneLocale *rl)
{

	l->__mbrtowc = _ascii_mbrtowc;
	l->__mbsinit = _ascii_mbsinit;
	l->__mbsnrtowcs = _ascii_mbsnrtowcs;
	l->__wcrtomb = _ascii_wcrtomb;
	l->__wcsnrtombs = _ascii_wcsnrtombs;
	l->runes = rl;
	l->__mb_cur_max = 1;
	l->__mb_sb_limit = 128;
	return(0);
}

static int
_ascii_mbsinit(const mbstate_t *ps __unused)
{

	/*
	 * Encoding is not state dependent - we are always in the
	 * initial state.
	 */
	return (1);
}

static size_t
_ascii_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps __unused)
{

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (0);
	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);
	if (*s & 0x80) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	if (pwc != NULL)
		*pwc = (unsigned char)*s;
	return (*s == '\0' ? 0 : 1);
}

static size_t
_ascii_wcrtomb(char * __restrict s, wchar_t wc,
    mbstate_t * __restrict ps __unused)
{

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc < 0 || wc > 127) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	*s = (unsigned char)wc;
	return (1);
}

static size_t
_ascii_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps __unused)
{
	const char *s;
	size_t nchr;

	if (dst == NULL) {
		for (s = *src; nms > 0 && *s != '\0'; s++, nms--) {
			if (*s & 0x80) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		return (s - *src);
	}

	s = *src;
	nchr = 0;
	while (len-- > 0 && nms-- > 0) {
		if (*s & 0x80) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		if ((*dst++ = (unsigned char)*s++) == L'\0') {
			*src = NULL;
			return (nchr);
		}
		nchr++;
	}
	*src = s;
	return (nchr);
}

static size_t
_ascii_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps __unused)
{
	const wchar_t *s;
	size_t nchr;

	if (dst == NULL) {
		for (s = *src; nwc > 0 && *s != L'\0'; s++, nwc--) {
			if (*s < 0 || *s > 127) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		return (s - *src);
	}

	s = *src;
	nchr = 0;
	while (len-- > 0 && nwc-- > 0) {
		if (*s < 0 || *s > 127) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		if ((*dst++ = *s++) == '\0') {
			*src = NULL;
			return (nchr);
		}
		nchr++;
	}
	*src = s;
	return (nchr);
}
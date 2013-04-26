
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

#include <sys/param.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

static size_t	_GB2312_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static int	_GB2312_mbsinit(const mbstate_t *);
static size_t	_GB2312_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);

typedef struct {
	int	count;
	u_char	bytes[2];
} _GB2312State;

int
_GB2312_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->runes = rl;
	l->__mbrtowc = _GB2312_mbrtowc;
	l->__wcrtomb = _GB2312_wcrtomb;
	l->__mbsinit = _GB2312_mbsinit;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

static int
_GB2312_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _GB2312State *)ps)->count == 0);
}

static __inline int
_GB2312_check(const char *str, size_t n)
{
	const u_char *s = (const u_char *)str;

	if (n == 0)
		/* Incomplete multibyte sequence */
		return (-2);
	if (s[0] >= 0xa1 && s[0] <= 0xfe) {
		if (n < 2)
			/* Incomplete multibyte sequence */
			return (-2);
		if (s[1] < 0xa1 || s[1] > 0xfe)
			/* Invalid multibyte sequence */
			return (-1);
		return (2);
	} else if (s[0] & 0x80) {
		/* Invalid multibyte sequence */
		return (-1);
	} 
	return (1);
}

static size_t
_GB2312_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_GB2312State *gs;
	wchar_t wc;
	int i, len, ocount;
	size_t ncopy;

	gs = (_GB2312State *)ps;

	if (gs->count < 0 || gs->count > sizeof(gs->bytes)) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	ncopy = MIN(MIN(n, MB_CUR_MAX), sizeof(gs->bytes) - gs->count);
	memcpy(gs->bytes + gs->count, s, ncopy);
	ocount = gs->count;
	gs->count += ncopy;
	s = (char *)gs->bytes;
	n = gs->count;

	if ((len = _GB2312_check(s, n)) < 0)
		return ((size_t)len);
	wc = 0;
	i = len;
	while (i-- > 0)
		wc = (wc << 8) | (unsigned char)*s++;
	if (pwc != NULL)
		*pwc = wc;
	gs->count = 0;
	return (wc == L'\0' ? 0 : len - ocount);
}

static size_t
_GB2312_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_GB2312State *gs;

	gs = (_GB2312State *)ps;

	if (gs->count != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc & 0x8000) {
		*s++ = (wc >> 8) & 0xff;
		*s = wc & 0xff;
		return (2);
	}
	*s = wc & 0xff;
	return (1);
}

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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)big5.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <errno.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

extern int __mb_sb_limit;

static size_t	_BIG5_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static int	_BIG5_mbsinit(const mbstate_t *);
static size_t	_BIG5_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);

typedef struct {
	wchar_t	ch;
} _BIG5State;

int
_BIG5_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = _BIG5_mbrtowc;
	l->__wcrtomb = _BIG5_wcrtomb;
	l->__mbsinit = _BIG5_mbsinit;
	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 128;
	return (0);
}

static int
_BIG5_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _BIG5State *)ps)->ch == 0);
}

static __inline int
_big5_check(u_int c)
{

	c &= 0xff;
	return ((c >= 0xa1 && c <= 0xfe) ? 2 : 1);
}

static size_t
_BIG5_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_BIG5State *bs;
	wchar_t wc;
	size_t len;

	bs = (_BIG5State *)ps;

	if ((bs->ch & ~0xFF) != 0) {
		/* Bad conversion state. */
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (bs->ch != 0) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (bs->ch << 8) | (*s & 0xFF);
		if (pwc != NULL)
			*pwc = wc;
		bs->ch = 0;
		return (1);
	}

	len = (size_t)_big5_check(*s);
	wc = *s++ & 0xff;
	if (len == 2) {
		if (n < 2) {
			/* Incomplete multibyte sequence */
			bs->ch = wc;
			return ((size_t)-2);
		}
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (*s++ & 0xff);
		if (pwc != NULL)
			*pwc = wc;
                return (2);
	} else {
		if (pwc != NULL)
			*pwc = wc;
		return (wc == L'\0' ? 0 : 1);
	}
}

static size_t
_BIG5_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_BIG5State *bs;

	bs = (_BIG5State *)ps;

	if (bs->ch != 0) {
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
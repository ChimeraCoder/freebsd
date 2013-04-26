
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
static char sccsid[] = "@(#)mskanji.c	1.0 (Phase One) 5/5/95";
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

static size_t	_MSKanji_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static int	_MSKanji_mbsinit(const mbstate_t *);
static size_t	_MSKanji_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);

typedef struct {
	wchar_t	ch;
} _MSKanjiState;

int
_MSKanji_init(struct xlocale_ctype *l, _RuneLocale *rl)
{

	l->__mbrtowc = _MSKanji_mbrtowc;
	l->__wcrtomb = _MSKanji_wcrtomb;
	l->__mbsinit = _MSKanji_mbsinit;
	l->runes = rl;
	l->__mb_cur_max = 2;
	l->__mb_sb_limit = 256;
	return (0);
}

static int
_MSKanji_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _MSKanjiState *)ps)->ch == 0);
}

static size_t
_MSKanji_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_MSKanjiState *ms;
	wchar_t wc;

	ms = (_MSKanjiState *)ps;

	if ((ms->ch & ~0xFF) != 0) {
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

	if (ms->ch != 0) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (ms->ch << 8) | (*s & 0xFF);
		if (pwc != NULL)
			*pwc = wc;
		ms->ch = 0;
		return (1);
	}
	wc = *s++ & 0xff;
	if ((wc > 0x80 && wc < 0xa0) || (wc >= 0xe0 && wc < 0xfd)) {
		if (n < 2) {
			/* Incomplete multibyte sequence */
			ms->ch = wc;
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
_MSKanji_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_MSKanjiState *ms;
	int len, i;

	ms = (_MSKanjiState *)ps;

	if (ms->ch != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	len = (wc > 0x100) ? 2 : 1;
	for (i = len; i-- > 0; )
		*s++ = wc >> (i << 3);
	return (len);
}
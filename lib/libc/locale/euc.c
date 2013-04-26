
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
static char sccsid[] = "@(#)euc.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/param.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <limits.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

extern int __mb_sb_limit;

static size_t	_EUC_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict);
static int	_EUC_mbsinit(const mbstate_t *);
static size_t	_EUC_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict);

typedef struct {
	int	count[4];
	wchar_t	bits[4];
	wchar_t	mask;
} _EucInfo;

typedef struct {
	wchar_t	ch;
	int	set;
	int	want;
} _EucState;

int
_EUC_init(struct xlocale_ctype *l, _RuneLocale *rl)
{
	_EucInfo *ei;
	int x, new__mb_cur_max;
	char *v, *e;

	if (rl->__variable == NULL)
		return (EFTYPE);

	v = (char *)rl->__variable;

	while (*v == ' ' || *v == '\t')
		++v;

	if ((ei = malloc(sizeof(_EucInfo))) == NULL)
		return (errno == 0 ? ENOMEM : errno);

	new__mb_cur_max = 0;
	for (x = 0; x < 4; ++x) {
		ei->count[x] = (int)strtol(v, &e, 0);
		if (v == e || !(v = e)) {
			free(ei);
			return (EFTYPE);
		}
		if (new__mb_cur_max < ei->count[x])
			new__mb_cur_max = ei->count[x];
		while (*v == ' ' || *v == '\t')
			++v;
		ei->bits[x] = (int)strtol(v, &e, 0);
		if (v == e || !(v = e)) {
			free(ei);
			return (EFTYPE);
		}
		while (*v == ' ' || *v == '\t')
			++v;
	}
	ei->mask = (int)strtol(v, &e, 0);
	if (v == e || !(v = e)) {
		free(ei);
		return (EFTYPE);
	}
	rl->__variable = ei;
	rl->__variable_len = sizeof(_EucInfo);
	l->runes = rl;
	l->__mb_cur_max = new__mb_cur_max;
	l->__mbrtowc = _EUC_mbrtowc;
	l->__wcrtomb = _EUC_wcrtomb;
	l->__mbsinit = _EUC_mbsinit;
	l->__mb_sb_limit = 256;
	return (0);
}

static int
_EUC_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _EucState *)ps)->want == 0);
}

#define	CEI	((_EucInfo *)(_CurrentRuneLocale->__variable))

#define	_SS2	0x008e
#define	_SS3	0x008f

#define	GR_BITS	0x80808080 /* XXX: to be fixed */

static __inline int
_euc_set(u_int c)
{

	c &= 0xff;
	return ((c & 0x80) ? c == _SS3 ? 3 : c == _SS2 ? 2 : 1 : 0);
}

static size_t
_EUC_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps)
{
	_EucState *es;
	int i, set, want;
	wchar_t wc;
	const char *os;

	es = (_EucState *)ps;

	if (es->want < 0 || es->want > MB_CUR_MAX || es->set < 0 ||
	    es->set > 3) {
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

	os = s;

	if (es->want == 0) {
		want = CEI->count[set = _euc_set(*s)];
		if (set == 2 || set == 3) {
			--want;
			if (--n == 0) {
				/* Incomplete multibyte sequence */
				es->set = set;
				es->want = want;
				es->ch = 0;
				return ((size_t)-2);
			}
			++s;
			if (*s == '\0') {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		wc = (unsigned char)*s++;
	} else {
		set = es->set;
		want = es->want;
		wc = es->ch;
	}
	for (i = (es->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (unsigned char)*s++;
	}
	if (i < want) {
		/* Incomplete multibyte sequence */
		es->set = set;
		es->want = want - i;
		es->ch = wc;
		return ((size_t)-2);
	}
	wc = (wc & ~CEI->mask) | CEI->bits[set];
	if (pwc != NULL)
		*pwc = wc;
	es->want = 0;
	return (wc == L'\0' ? 0 : s - os);
}

static size_t
_EUC_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	_EucState *es;
	wchar_t m, nm;
	int i, len;

	es = (_EucState *)ps;

	if (es->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	m = wc & CEI->mask;
	nm = wc & ~m;

	if (m == CEI->bits[1]) {
CodeSet1:
		/* Codeset 1: The first byte must have 0x80 in it. */
		i = len = CEI->count[1];
		while (i-- > 0)
			*s++ = (nm >> (i << 3)) | 0x80;
	} else {
		if (m == CEI->bits[0])
			i = len = CEI->count[0];
		else if (m == CEI->bits[2]) {
			i = len = CEI->count[2];
			*s++ = _SS2;
			--i;
			/* SS2 designates G2 into GR */
			nm |= GR_BITS;
		} else if (m == CEI->bits[3]) {
			i = len = CEI->count[3];
			*s++ = _SS3;
			--i;
			/* SS3 designates G3 into GR */
			nm |= GR_BITS;
		} else
			goto CodeSet1;	/* Bletch */
		while (i-- > 0)
			*s++ = (nm >> (i << 3)) & 0xff;
	}
	return (len);
}
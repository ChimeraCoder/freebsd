
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
#if 0
__FBSDID("FreeBSD: src/lib/libc/string/strxfrm.c,v 1.15 2002/09/06 11:24:06 tjr Exp ");
#endif
__FBSDID("$FreeBSD$");

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "collate.h"

static char *__mbsdup(const wchar_t *);

/*
 * Placeholder wcsxfrm() implementation. See wcscoll.c for a description of
 * the logic used.
 */
size_t
wcsxfrm_l(wchar_t * __restrict dest, const wchar_t * __restrict src, size_t len, locale_t locale)
{
	int prim, sec, l;
	size_t slen;
	char *mbsrc, *s, *ss;
	FIX_LOCALE(locale);
	struct xlocale_collate *table =
		(struct xlocale_collate*)locale->components[XLC_COLLATE];

	if (*src == L'\0') {
		if (len != 0)
			*dest = L'\0';
		return (0);
	}

	if (table->__collate_load_error || MB_CUR_MAX > 1) {
		slen = wcslen(src);
		if (len > 0) {
			if (slen < len)
				wcscpy(dest, src);
			else {
				wcsncpy(dest, src, len - 1);
				dest[len - 1] = L'\0';
			}
		}
		return (slen);
	}

	mbsrc = __mbsdup(src);
	slen = 0;
	prim = sec = 0;
	ss = s = __collate_substitute(table, mbsrc);
	while (*s != '\0') {
		while (*s != '\0' && prim == 0) {
			__collate_lookup(table, s, &l, &prim, &sec);
			s += l;
		}
		if (prim != 0) {
			if (len > 1) {
				*dest++ = (wchar_t)prim;
				len--;
			}
			slen++;
			prim = 0;
		}
	}
	free(ss);
	free(mbsrc);
	if (len != 0)
		*dest = L'\0';

	return (slen);
}
size_t
wcsxfrm(wchar_t * __restrict dest, const wchar_t * __restrict src, size_t len)
{
	return wcsxfrm_l(dest, src, len, __get_locale());
}

static char *
__mbsdup(const wchar_t *ws)
{
	static const mbstate_t initial;
	mbstate_t st;
	const wchar_t *wcp;
	size_t len;
	char *mbs;

	wcp = ws;
	st = initial;
	if ((len = wcsrtombs(NULL, &wcp, 0, &st)) == (size_t)-1)
		return (NULL);
	if ((mbs = malloc(len + 1)) == NULL)
		return (NULL);
	st = initial;
	wcsrtombs(mbs, &ws, len + 1, &st);

	return (mbs);
}

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

#include <runetype.h>
#include <wchar.h>
#include <wctype.h>
#include "mblocal.h"

wint_t
nextwctype_l(wint_t wc, wctype_t wct, locale_t locale)
{
	size_t lim;
	FIX_LOCALE(locale);
	_RuneLocale *runes = XLOCALE_CTYPE(locale)->runes;
	_RuneRange *rr = &runes->__runetype_ext;
	_RuneEntry *base, *re;
	int noinc;

	noinc = 0;
	if (wc < _CACHED_RUNES) {
		wc++;
		while (wc < _CACHED_RUNES) {
			if (runes->__runetype[wc] & wct)
				return (wc);
			wc++;
		}
		wc--;
	}
	if (rr->__ranges != NULL && wc < rr->__ranges[0].__min) {
		wc = rr->__ranges[0].__min;
		noinc = 1;
	}

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= wc && wc <= re->__max)
			goto found;
		else if (wc > re->__max) {
			base = re + 1;
			lim--;
		}
	}
	return (-1);
found:
	if (!noinc)
		wc++;
	if (re->__min <= wc && wc <= re->__max) {
		if (re->__types != NULL) {
			for (; wc <= re->__max; wc++)
				if (re->__types[wc - re->__min] & wct)
					return (wc);
		} else if (re->__map & wct)
			return (wc);
	}
	while (++re < rr->__ranges + rr->__nranges) {
		wc = re->__min;
		if (re->__types != NULL) {
			for (; wc <= re->__max; wc++)
				if (re->__types[wc - re->__min] & wct)
					return (wc);
		} else if (re->__map & wct)
			return (wc);
	}
	return (-1);
}
wint_t
nextwctype(wint_t wc, wctype_t wct)
{
	return nextwctype_l(wc, wct, __get_locale());
}
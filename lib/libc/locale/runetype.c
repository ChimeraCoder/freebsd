
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

#include <ctype.h>
#include <stdio.h>
#include <runetype.h>
#include <wchar.h>
#include "mblocal.h"

unsigned long
___runetype_l(__ct_rune_t c, locale_t locale)
{
	size_t lim;
	FIX_LOCALE(locale);
	_RuneRange *rr = &(XLOCALE_CTYPE(locale)->runes->__runetype_ext);
	_RuneEntry *base, *re;

	if (c < 0 || c == EOF)
		return(0L);

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= c && c <= re->__max) {
			if (re->__types)
			    return(re->__types[c - re->__min]);
			else
			    return(re->__map);
		} else if (c > re->__max) {
			base = re + 1;
			lim--;
		}
	}

	return(0L);
}
unsigned long
___runetype(__ct_rune_t c)
{
	return ___runetype_l(c, __get_locale());
}

int ___mb_cur_max(void)
{
	return XLOCALE_CTYPE(__get_locale())->__mb_cur_max;
}
int ___mb_cur_max_l(locale_t locale)
{
	FIX_LOCALE(locale);
	return XLOCALE_CTYPE(locale)->__mb_cur_max;
}

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

__ct_rune_t
___toupper_l(c, l)
	__ct_rune_t c;
	locale_t l;
{
	size_t lim;
	FIX_LOCALE(l);
	_RuneRange *rr = &XLOCALE_CTYPE(l)->runes->__mapupper_ext;
	_RuneEntry *base, *re;

	if (c < 0 || c == EOF)
		return(c);

	/* Binary search -- see bsearch.c for explanation. */
	base = rr->__ranges;
	for (lim = rr->__nranges; lim != 0; lim >>= 1) {
		re = base + (lim >> 1);
		if (re->__min <= c && c <= re->__max)
		{
			return (re->__map + c - re->__min);
		}
		else if (c > re->__max) {
			base = re + 1;
			lim--;
		}
	}

	return(c);
}
__ct_rune_t
___toupper(c)
	__ct_rune_t c;
{
	return ___toupper_l(c, __get_locale());
}
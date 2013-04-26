
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

#include <stdio.h>
#include <wchar.h>
#include "mblocal.h"

wint_t
btowc_l(int c, locale_t l)
{
	static const mbstate_t initial;
	mbstate_t mbs = initial;
	char cc;
	wchar_t wc;
	FIX_LOCALE(l);

	if (c == EOF)
		return (WEOF);
	/*
	 * We expect mbrtowc() to return 0 or 1, hence the check for n > 1
	 * which detects error return values as well as "impossible" byte
	 * counts.
	 */
	cc = (char)c;
	if (XLOCALE_CTYPE(l)->__mbrtowc(&wc, &cc, 1, &mbs) > 1)
		return (WEOF);
	return (wc);
}
wint_t
btowc(int c)
{
	return btowc_l(c, __get_locale());
}
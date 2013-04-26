
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

#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include "xlocale_private.h"

/*
 * See wcstod() for comments as to the logic used.
 */
long double
wcstold_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	long double val;
	char *buf, *end;
	const wchar_t *wcp = nptr;
	size_t len;
	size_t spaces = 0;
	FIX_LOCALE(locale);

	while (iswspace_l(*wcp, locale)) {
		wcp++;
		spaces++;
	}

	wcp = nptr;
	mbs = initial;
	if ((len = wcsrtombs_l(NULL, &wcp, 0, &mbs, locale)) == (size_t)-1) {
		if (endptr != NULL)
			*endptr = (wchar_t *)nptr;
		return (0.0);
	}
	if ((buf = malloc(len + 1)) == NULL)
		return (0.0);
	mbs = initial;
	wcsrtombs_l(buf, &wcp, len + 1, &mbs, locale);

	val = strtold_l(buf, &end, locale);

	if (endptr != NULL) {
		/* XXX Assume each wide char is one byte. */
		*endptr = (wchar_t *)nptr + (end - buf);
		if (buf != end)
			*endptr += spaces;
	}

	free(buf);

	return (val);
}
long double
wcstold(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstold_l(nptr, endptr, __get_locale());
}
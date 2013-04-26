
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
float
wcstof_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	float val;
	char *buf, *end;
	const wchar_t *wcp;
	size_t len;
	FIX_LOCALE(locale);

	while (iswspace_l(*nptr, locale))
		nptr++;

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

	val = strtof_l(buf, &end, locale);

	if (endptr != NULL)
		*endptr = (wchar_t *)nptr + (end - buf);

	free(buf);

	return (val);
}
float
wcstof(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstof_l(nptr, endptr, __get_locale());
}
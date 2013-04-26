
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
#include "mblocal.h"

int
mblen_l(const char *s, size_t n, locale_t locale)
{
	static const mbstate_t initial;
	size_t rval;
	FIX_LOCALE(locale);

	if (s == NULL) {
		/* No support for state dependent encodings. */
		locale->mblen = initial;
		return (0);
	}
	rval = XLOCALE_CTYPE(locale)->__mbrtowc(NULL, s, n, &locale->mblen);
	if (rval == (size_t)-1 || rval == (size_t)-2)
		return (-1);
	return ((int)rval);
}

int
mblen(const char *s, size_t n)
{
	return mblen_l(s, n, __get_locale());
}
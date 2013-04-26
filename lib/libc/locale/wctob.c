
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

#include <limits.h>
#include <stdio.h>
#include <wchar.h>
#include "mblocal.h"

int
wctob_l(wint_t c, locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs = initial;
	char buf[MB_LEN_MAX];
	FIX_LOCALE(locale);

	if (c == WEOF || XLOCALE_CTYPE(locale)->__wcrtomb(buf, c, &mbs) != 1)
		return (EOF);
	return ((unsigned char)*buf);
}
int
wctob(wint_t c)
{
	return wctob_l(c, __get_locale());
}
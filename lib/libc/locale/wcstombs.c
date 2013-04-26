
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
#include <stdlib.h>
#include <wchar.h>
#include "mblocal.h"

size_t
wcstombs_l(char * __restrict s, const wchar_t * __restrict pwcs, size_t n,
		locale_t locale)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	const wchar_t *pwcsp;
	FIX_LOCALE(locale);

	mbs = initial;
	pwcsp = pwcs;
	return (XLOCALE_CTYPE(locale)->__wcsnrtombs(s, &pwcsp, SIZE_T_MAX, n, &mbs));
}
size_t
wcstombs(char * __restrict s, const wchar_t * __restrict pwcs, size_t n)
{
	return wcstombs_l(s, pwcs, n, __get_locale());
}
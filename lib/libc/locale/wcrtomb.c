
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

#include <wchar.h>
#include "mblocal.h"

size_t
wcrtomb_l(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps,
		locale_t locale)
{
	FIX_LOCALE(locale);
	if (ps == NULL)
		ps = &locale->wcrtomb;
	return (XLOCALE_CTYPE(locale)->__wcrtomb(s, wc, ps));
}

size_t
wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps)
{
	return wcrtomb_l(s, wc, ps, __get_locale());
}
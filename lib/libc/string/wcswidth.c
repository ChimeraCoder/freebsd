
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
#include "xlocale_private.h"

int
wcswidth_l(const wchar_t *pwcs, size_t n, locale_t locale)
{
	wchar_t wc;
	int len, l;
	FIX_LOCALE(locale);

	len = 0;
	while (n-- > 0 && (wc = *pwcs++) != L'\0') {
		if ((l = wcwidth_l(wc, locale)) < 0)
			return (-1);
		len += l;
	}
	return (len);
}

int
wcswidth(const wchar_t *pwcs, size_t n)
{
	return wcswidth_l(pwcs, n, __get_locale());
}
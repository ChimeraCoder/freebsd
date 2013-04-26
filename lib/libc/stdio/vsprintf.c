
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)vsprintf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include <limits.h>
#include "local.h"
#include "xlocale_private.h"

int
vsprintf_l(char * __restrict str, locale_t locale,
		const char * __restrict fmt, __va_list ap)
{
	int ret;
	FILE f = FAKE_FILE;
	FIX_LOCALE(locale);

	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = INT_MAX;
	ret = __vfprintf(&f, locale, fmt, ap);
	*f._p = 0;
	return (ret);
}
int
vsprintf(char * __restrict str, const char * __restrict fmt, __va_list ap)
{
	return vsprintf_l(str, __get_locale(), fmt, ap);
}
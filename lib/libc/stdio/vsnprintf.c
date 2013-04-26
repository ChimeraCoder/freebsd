
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
static char sccsid[] = "@(#)vsnprintf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include "local.h"
#include "xlocale_private.h"

int
vsnprintf_l(char * __restrict str, size_t n, locale_t locale, 
		const char * __restrict fmt, __va_list ap)
{
	size_t on;
	int ret;
	char dummy[2];
	FILE f = FAKE_FILE;
	FIX_LOCALE(locale);

	on = n;
	if (n != 0)
		n--;
	if (n > INT_MAX) {
		errno = EOVERFLOW;
		*str = '\0';
		return (EOF);
	}
	/* Stdio internals do not deal correctly with zero length buffer */
	if (n == 0) {
		if (on > 0)
	  		*str = '\0';
		str = dummy;
		n = 1;
	}
	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = n;
	ret = __vfprintf(&f, locale, fmt, ap);
	if (on > 0)
		*f._p = '\0';
	return (ret);
}
int
vsnprintf(char * __restrict str, size_t n, const char * __restrict fmt,
    __va_list ap)
{
	return vsnprintf_l(str, n, __get_locale(), fmt, ap);
}
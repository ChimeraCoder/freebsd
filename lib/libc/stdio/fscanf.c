
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
static char sccsid[] = "@(#)fscanf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <stdio.h>
#include <stdarg.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "xlocale_private.h"

int
fscanf(FILE * __restrict fp, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	FLOCKFILE(fp);
	ret = __svfscanf(fp, __get_locale(), fmt, ap);
	va_end(ap);
	FUNLOCKFILE(fp);
	return (ret);
}
int
fscanf_l(FILE * __restrict fp, locale_t locale, char const * __restrict fmt, ...)
{
	int ret;
	va_list ap;
	FIX_LOCALE(locale);

	va_start(ap, fmt);
	FLOCKFILE(fp);
	ret = __svfscanf(fp, locale, fmt, ap);
	va_end(ap);
	FUNLOCKFILE(fp);
	return (ret);
}
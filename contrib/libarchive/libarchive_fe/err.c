
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

#include "lafe_platform.h"
__FBSDID("$FreeBSD$");

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "err.h"

static void lafe_vwarnc(int, const char *, va_list) __LA_PRINTFLIKE(2, 0);

const char *lafe_progname;

static void
lafe_vwarnc(int code, const char *fmt, va_list ap)
{
	fprintf(stderr, "%s: ", lafe_progname);
	vfprintf(stderr, fmt, ap);
	if (code != 0)
		fprintf(stderr, ": %s", strerror(code));
	fprintf(stderr, "\n");
}

void
lafe_warnc(int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	lafe_vwarnc(code, fmt, ap);
	va_end(ap);
}

void
lafe_errc(int eval, int code, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	lafe_vwarnc(code, fmt, ap);
	va_end(ap);
	exit(eval);
}
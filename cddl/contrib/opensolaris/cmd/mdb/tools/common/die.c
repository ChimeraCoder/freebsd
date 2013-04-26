
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <elf.h>

#include <util.h>

void
die(char *format, ...)
{
	va_list ap;
	int err = errno;
#if !defined(sun)
	const char *progname = getprogname();
#endif

	(void) fprintf(stderr, "%s: ", progname);

	va_start(ap, format);
	/* LINTED - variable format specifier */
	(void) vfprintf(stderr, format, ap);
	va_end(ap);

	if (format[strlen(format) - 1] != '\n')
		(void) fprintf(stderr, ": %s\n", strerror(err));

#if defined(__FreeBSD__)
	exit(0);
#else
	exit(1);
#endif
}

void
elfdie(char *format, ...)
{
	va_list ap;
#if !defined(sun)
	const char *progname = getprogname();
#endif

	(void) fprintf(stderr, "%s: ", progname);

	va_start(ap, format);
	/* LINTED - variable format specifier */
	(void) vfprintf(stderr, format, ap);
	va_end(ap);

	if (format[strlen(format) - 1] != '\n')
		(void) fprintf(stderr, ": %s\n", elf_errmsg(elf_errno()));

#if defined(__FreeBSD__)
	exit(0);
#else
	exit(1);
#endif
}
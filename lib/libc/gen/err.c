
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
static char sccsid[] = "@(#)err.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "un-namespace.h"

#include "libc_private.h"

static FILE *err_file; /* file to use for error output */
static void (*err_exit)(int);

/*
 * This is declared to take a `void *' so that the caller is not required
 * to include <stdio.h> first.  However, it is really a `FILE *', and the
 * manual page documents it as such.
 */
void
err_set_file(void *fp)
{
	if (fp)
		err_file = fp;
	else
		err_file = stderr;
}

void
err_set_exit(void (*ef)(int))
{
	err_exit = ef;
}

__weak_reference(_err, err);

void
_err(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrc(eval, errno, fmt, ap);
	va_end(ap);
}

void
verr(eval, fmt, ap)
	int eval;
	const char *fmt;
	va_list ap;
{
	verrc(eval, errno, fmt, ap);
}

void
errc(int eval, int code, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrc(eval, code, fmt, ap);
	va_end(ap);
}

void
verrc(int eval, int code, const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	fprintf(err_file, "%s: ", _getprogname());
	if (fmt != NULL) {
		vfprintf(err_file, fmt, ap);
		fprintf(err_file, ": ");
	}
	fprintf(err_file, "%s\n", strerror(code));
	if (err_exit)
		err_exit(eval);
	exit(eval);
}

void
errx(int eval, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verrx(eval, fmt, ap);
	va_end(ap);
}

void
verrx(int eval, const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	fprintf(err_file, "%s: ", _getprogname());
	if (fmt != NULL)
		vfprintf(err_file, fmt, ap);
	fprintf(err_file, "\n");
	if (err_exit)
		err_exit(eval);
	exit(eval);
}

__weak_reference(_warn, warn);

void
_warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnc(errno, fmt, ap);
	va_end(ap);
}

void
vwarn(const char *fmt, va_list ap)
{
	vwarnc(errno, fmt, ap);
}

void
warnc(int code, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnc(code, fmt, ap);
	va_end(ap);
}

void
vwarnc(int code, const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	fprintf(err_file, "%s: ", _getprogname());
	if (fmt != NULL) {
		vfprintf(err_file, fmt, ap);
		fprintf(err_file, ": ");
	}
	fprintf(err_file, "%s\n", strerror(code));
}

void
warnx(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vwarnx(fmt, ap);
	va_end(ap);
}

void
vwarnx(const char *fmt, va_list ap)
{
	if (err_file == 0)
		err_set_file((FILE *)0);
	fprintf(err_file, "%s: ", _getprogname());
	if (fmt != NULL)
		vfprintf(err_file, fmt, ap);
	fprintf(err_file, "\n");
}
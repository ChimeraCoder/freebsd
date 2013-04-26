
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

/* $Id: error.c,v 1.16 2001/08/08 22:54:49 gson Exp $ */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <isc/error.h>
#include <isc/msgs.h>

static void
default_unexpected_callback(const char *, int, const char *, va_list)
     ISC_FORMAT_PRINTF(3, 0);

static void
default_fatal_callback(const char *, int, const char *, va_list)
     ISC_FORMAT_PRINTF(3, 0);

static isc_errorcallback_t unexpected_callback = default_unexpected_callback;
static isc_errorcallback_t fatal_callback = default_fatal_callback;

void
isc_error_setunexpected(isc_errorcallback_t cb) {
	if (cb == NULL)
		unexpected_callback = default_unexpected_callback;
	else
		unexpected_callback = cb;
}

void
isc_error_setfatal(isc_errorcallback_t cb) {
	if (cb == NULL)
		fatal_callback = default_fatal_callback;
	else
		fatal_callback = cb;
}

void
isc_error_unexpected(const char *file, int line, const char *format, ...) {
	va_list args;

	va_start(args, format);
	(unexpected_callback)(file, line, format, args);
	va_end(args);
}

void
isc_error_fatal(const char *file, int line, const char *format, ...) {
	va_list args;

	va_start(args, format);
	(fatal_callback)(file, line, format, args);
	va_end(args);
	abort();
}

void
isc_error_runtimecheck(const char *file, int line, const char *expression) {
	isc_error_fatal(file, line, "RUNTIME_CHECK(%s) %s", expression,
			isc_msgcat_get(isc_msgcat, ISC_MSGSET_GENERAL,
				       ISC_MSG_FAILED, "failed"));
}

static void
default_unexpected_callback(const char *file, int line, const char *format,
			    va_list args)
{
	fprintf(stderr, "%s:%d: ", file, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	fflush(stderr);
}

static void
default_fatal_callback(const char *file, int line, const char *format,
		       va_list args)
{
	fprintf(stderr, "%s:%d: %s: ", file, line,
		isc_msgcat_get(isc_msgcat, ISC_MSGSET_GENERAL,
			       ISC_MSG_FATALERROR, "fatal error"));
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	fflush(stderr);
}
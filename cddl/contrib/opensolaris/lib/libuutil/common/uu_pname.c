
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

#include "libuutil_common.h"

#include <libintl.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <wchar.h>
#include <unistd.h>

static const char PNAME_FMT[] = "%s: ";
static const char ERRNO_FMT[] = ": %s\n";

static const char *pname;

static void
uu_die_internal(int status, const char *format, va_list alist) __NORETURN;

int uu_exit_ok_value = EXIT_SUCCESS;
int uu_exit_fatal_value = EXIT_FAILURE;
int uu_exit_usage_value = 2;

int *
uu_exit_ok(void)
{
	return (&uu_exit_ok_value);
}

int *
uu_exit_fatal(void)
{
	return (&uu_exit_fatal_value);
}

int *
uu_exit_usage(void)
{
	return (&uu_exit_usage_value);
}

void
uu_alt_exit(int profile)
{
	switch (profile) {
	case UU_PROFILE_DEFAULT:
		uu_exit_ok_value = EXIT_SUCCESS;
		uu_exit_fatal_value = EXIT_FAILURE;
		uu_exit_usage_value = 2;
		break;
	case UU_PROFILE_LAUNCHER:
		uu_exit_ok_value = EXIT_SUCCESS;
		uu_exit_fatal_value = 124;
		uu_exit_usage_value = 125;
		break;
	}
}

static void
uu_warn_internal(int err, const char *format, va_list alist)
{
	if (pname != NULL)
		(void) fprintf(stderr, PNAME_FMT, pname);

	(void) vfprintf(stderr, format, alist);

	if (strrchr(format, '\n') == NULL)
		(void) fprintf(stderr, ERRNO_FMT, strerror(err));
}

void
uu_vwarn(const char *format, va_list alist)
{
	uu_warn_internal(errno, format, alist);
}

/*PRINTFLIKE1*/
void
uu_warn(const char *format, ...)
{
	va_list alist;
	va_start(alist, format);
	uu_warn_internal(errno, format, alist);
	va_end(alist);
}

static void
uu_die_internal(int status, const char *format, va_list alist)
{
	uu_warn_internal(errno, format, alist);
#ifdef DEBUG
	{
		char *cp;

		if (!issetugid()) {
			cp = getenv("UU_DIE_ABORTS");
			if (cp != NULL && *cp != '\0')
				abort();
		}
	}
#endif
	exit(status);
}

void
uu_vdie(const char *format, va_list alist)
{
	uu_die_internal(UU_EXIT_FATAL, format, alist);
}

/*PRINTFLIKE1*/
void
uu_die(const char *format, ...)
{
	va_list alist;
	va_start(alist, format);
	uu_die_internal(UU_EXIT_FATAL, format, alist);
	va_end(alist);
}

void
uu_vxdie(int status, const char *format, va_list alist)
{
	uu_die_internal(status, format, alist);
}

/*PRINTFLIKE2*/
void
uu_xdie(int status, const char *format, ...)
{
	va_list alist;
	va_start(alist, format);
	uu_die_internal(status, format, alist);
	va_end(alist);
}

const char *
uu_setpname(char *arg0)
{
	/*
	 * Having a NULL argv[0], while uncommon, is possible.  It
	 * makes more sense to handle this event in uu_setpname rather
	 * than in each of its consumers.
	 */
	if (arg0 == NULL) {
		pname = "unknown_command";
		return (pname);
	}

	/*
	 * Guard against '/' at end of command invocation.
	 */
	for (;;) {
		char *p = strrchr(arg0, '/');
		if (p == NULL) {
			pname = arg0;
			break;
		} else {
			if (*(p + 1) == '\0') {
				*p = '\0';
				continue;
			}

			pname = p + 1;
			break;
		}
	}

	return (pname);
}

const char *
uu_getpname(void)
{
	return (pname);
}

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

int openpam_debug = 0;

#if !defined(openpam_log)

/*
 * OpenPAM extension
 *
 * Log a message through syslog
 */

void
openpam_log(int level, const char *fmt, ...)
{
	va_list ap;
	int priority;

	switch (level) {
	case PAM_LOG_LIBDEBUG:
	case PAM_LOG_DEBUG:
		if (!openpam_debug)
			return;
		priority = LOG_DEBUG;
		break;
	case PAM_LOG_VERBOSE:
		priority = LOG_INFO;
		break;
	case PAM_LOG_NOTICE:
		priority = LOG_NOTICE;
		break;
	case PAM_LOG_ERROR:
	default:
		priority = LOG_ERR;
		break;
	}
	va_start(ap, fmt);
	vsyslog(priority, fmt, ap);
	va_end(ap);
}

#else

void
_openpam_log(int level, const char *func, const char *fmt, ...)
{
	va_list ap;
	char *format;
	int priority;
	int serrno;

	switch (level) {
	case PAM_LOG_LIBDEBUG:
	case PAM_LOG_DEBUG:
		if (!openpam_debug)
			return;
		priority = LOG_DEBUG;
		break;
	case PAM_LOG_VERBOSE:
		priority = LOG_INFO;
		break;
	case PAM_LOG_NOTICE:
		priority = LOG_NOTICE;
		break;
	case PAM_LOG_ERROR:
	default:
		priority = LOG_ERR;
		break;
	}
	va_start(ap, fmt);
	serrno = errno;
	if (asprintf(&format, "in %s(): %s", func, fmt) > 0) {
		errno = serrno;
		vsyslog(priority, format, ap);
		FREE(format);
	} else {
		errno = serrno;
		vsyslog(priority, fmt, ap);
	}
	va_end(ap);
}

#endif

/**
 * The =openpam_log function logs messages using =syslog.
 * It is primarily intended for internal use by the library and modules.
 *
 * The =level argument indicates the importance of the message.
 * The following levels are defined:
 *
 *	=PAM_LOG_LIBDEBUG:
 *		Debugging messages.
 *		For internal use only.
 *	=PAM_LOG_DEBUG:
 *		Debugging messages.
 *		These messages are normally not logged unless the global
 *		integer variable :openpam_debug is set to a non-zero
 *		value, in which case they are logged with a =syslog
 *		priority of =LOG_DEBUG.
 *	=PAM_LOG_VERBOSE:
 *		Information about the progress of the authentication
 *		process, or other non-essential messages.
 *		These messages are logged with a =syslog priority of
 *		=LOG_INFO.
 *	=PAM_LOG_NOTICE:
 *		Messages relating to non-fatal errors.
 *		These messages are logged with a =syslog priority of
 *		=LOG_NOTICE.
 *	=PAM_LOG_ERROR:
 *		Messages relating to serious errors.
 *		These messages are logged with a =syslog priority of
 *		=LOG_ERR.
 *
 * The remaining arguments are a =printf format string and the
 * corresponding arguments.
 */

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

/*
 * error logging/reporting facilities
 * stolen from /usr/libexec/mail.local via ypserv
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include "yp_extern.h"

int debug;
extern int _rpcpmstart;

extern char *progname;

static void __verr(const char *fmt, va_list ap) __printflike(1, 0);

static void __verr(const char *fmt, va_list ap)
{
	if (debug && !_rpcpmstart) {
		fprintf(stderr,"%s: ",progname);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
	} else {
		vsyslog(LOG_NOTICE, fmt, ap);
	}
}

void
yp_error(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	__verr(fmt,ap);
	va_end(ap);
}
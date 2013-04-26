
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

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include "log.h"

void
__log_msg(int level, const char *sender, const char *message, ...)
{
	va_list ap;
	char	*fmessage;

	fmessage = NULL;
	va_start(ap, message);
	vasprintf(&fmessage, message, ap);
	va_end(ap);
	assert(fmessage != NULL);

	printf("M%d from %s: %s\n", level, sender, fmessage);
#ifndef NO_SYSLOG
	if (level == 0)
		syslog(LOG_INFO, "nscd message (from %s): %s", sender,
		fmessage);
#endif
	free(fmessage);
}

void
__log_err(int level, const char *sender, const char *error, ...)
{
	va_list ap;
	char	*ferror;

	ferror = NULL;
	va_start(ap, error);
	vasprintf(&ferror, error, ap);
	va_end(ap);
	assert(ferror != NULL);

	printf("E%d from %s: %s\n", level, sender, ferror);

#ifndef NO_SYSLOG
	if (level == 0)
		syslog(LOG_ERR, "nscd error (from %s): %s", sender, ferror);
#endif
	free(ferror);
}

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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <fcntl.h>

#include "pw.h"

static FILE    *logfile = NULL;

void
pw_log(struct userconf * cnf, int mode, int which, char const * fmt,...)
{
	if (cnf->logfile && *cnf->logfile) {
		if (logfile == NULL) {	/* With umask==0 we need to control file access modes on create */
			int             fd = open(cnf->logfile, O_WRONLY | O_CREAT | O_APPEND, 0600);

			if (fd != -1)
				logfile = fdopen(fd, "a");
		}
		if (logfile != NULL) {
			va_list         argp;
			time_t          now = time(NULL);
			struct tm      *t = localtime(&now);
			char            nfmt[256];
			const char     *name;

			if ((name = getenv("LOGNAME")) == NULL && (name = getenv("USER")) == NULL)
				name = "unknown";
			/* ISO 8601 International Standard Date format */
			strftime(nfmt, sizeof nfmt, "%Y-%m-%d %T ", t);
			sprintf(nfmt + strlen(nfmt), "[%s:%s%s] %s\n", name, Which[which], Modes[mode], fmt);
			va_start(argp, fmt);
			vfprintf(logfile, nfmt, argp);
			va_end(argp);
			fflush(logfile);
		}
	}
}
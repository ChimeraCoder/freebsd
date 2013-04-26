
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)startdaemon.c	8.2 (Berkeley) 4/17/94";
#endif /* not lint */
#endif

#include "lp.cdefs.h"		/* A cross-platform version of <sys/cdefs.h> */
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>

#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "lp.h"
#include "pathnames.h"

/*
 * Tell the printer daemon that there are new files in the spool directory.
 */

int
startdaemon(const struct printer *pp)
{
	struct sockaddr_un un;
	register int s, n;
	int connectres;
	char c;

	s = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (s < 0) {
		warn("socket");
		return(0);
	}
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_LOCAL;
	strcpy(un.sun_path, _PATH_SOCKETNAME);
#ifndef SUN_LEN
#define SUN_LEN(unp) (strlen((unp)->sun_path) + 2)
#endif
	PRIV_START
	connectres = connect(s, (struct sockaddr *)&un, SUN_LEN(&un));
	PRIV_END
	if (connectres < 0) {
		warn("Unable to connect to %s", _PATH_SOCKETNAME);
		warnx("Check to see if the master 'lpd' process is running.");
		(void) close(s);
		return(0);
	}

	/*
	 * Avoid overruns without putting artificial limitations on 
	 * the length.
	 */
	if (writel(s, "\1", pp->printer, "\n", (char *)0) <= 0) {
		warn("write");
		(void) close(s);
		return(0);
	}
	if (read(s, &c, 1) == 1) {
		if (c == '\0') {		/* everything is OK */
			(void) close(s);
			return(1);
		}
		putchar(c);
	}
	while ((n = read(s, &c, 1)) > 0)
		putchar(c);
	(void) close(s);
	return(0);
}
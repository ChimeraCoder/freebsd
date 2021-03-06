
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
static const char copyright[] =
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)talkd.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

/*
 * The top level of the daemon, the format is heavily borrowed
 * from rwhod.c. Basically: find out who and where you are;
 * disconnect all descriptors and ttys, and then endless
 * loop on waiting for and processing requests
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <protocols/talkd.h>
#include <err.h>
#include <errno.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "extern.h"

static CTL_MSG		request;
static CTL_RESPONSE	response;

int			debug = 0;
static long		lastmsgtime;

char			hostname[MAXHOSTNAMELEN];

#define TIMEOUT 30
#define MAXIDLE 120

int
main(int argc, char *argv[])
{
	register CTL_MSG *mp = &request;
	int cc;
	struct sockaddr ctl_addr;

#ifdef NOTDEF
	/*
	 * removed so ntalkd can run in tty sandbox
	 */
	if (getuid())
		errx(1, "getuid: not super-user");
#endif
	openlog("talkd", LOG_PID, LOG_DAEMON);
	if (gethostname(hostname, sizeof(hostname) - 1) < 0) {
		syslog(LOG_ERR, "gethostname: %m");
		_exit(1);
	}
	hostname[sizeof(hostname) - 1] = '\0';
	if (chdir(_PATH_DEV) < 0) {
		syslog(LOG_ERR, "chdir: %s: %m", _PATH_DEV);
		_exit(1);
	}
	if (argc > 1 && strcmp(argv[1], "-d") == 0)
		debug = 1;
	signal(SIGALRM, timeout);
	alarm(TIMEOUT);
	for (;;) {
		cc = recv(0, (char *)mp, sizeof(*mp), 0);
		if (cc != sizeof (*mp)) {
			if (cc < 0 && errno != EINTR)
				syslog(LOG_WARNING, "recv: %m");
			continue;
		}
		lastmsgtime = time(0);
		(void)memcpy(&ctl_addr, &mp->ctl_addr, sizeof(ctl_addr));
		ctl_addr.sa_family = ntohs(mp->ctl_addr.sa_family);
		ctl_addr.sa_len = sizeof(ctl_addr);
		process_request(mp, &response);
		/* can block here, is this what I want? */
		cc = sendto(STDIN_FILENO, (char *)&response,
		    sizeof(response), 0, &ctl_addr, sizeof(ctl_addr));
		if (cc != sizeof (response))
			syslog(LOG_WARNING, "sendto: %m");
	}
}

void
timeout(int sig __unused)
{
	int save_errno = errno;

	if (time(0) - lastmsgtime >= MAXIDLE)
		_exit(0);
	alarm(TIMEOUT);
	errno = save_errno;
}
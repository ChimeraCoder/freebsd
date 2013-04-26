
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

/*
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define NANOSEC	1000000000

int
main(int argc, char **argv)
{
	struct sigevent ev;
	struct itimerspec ts;
	sigset_t set;
	timer_t tid;
	char *cmd = argv[0];
	int sig;

	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGUSR1;

	if (timer_create(CLOCK_REALTIME, &ev, &tid) == -1) {
		(void) fprintf(stderr, "%s: cannot create CLOCK_HIGHRES "
		    "timer: %s\n", cmd, strerror(errno));
		exit(EXIT_FAILURE);
	}

	(void) sigemptyset(&set);
	(void) sigaddset(&set, SIGUSR1);
	(void) sigprocmask(SIG_BLOCK, &set, NULL);

	ts.it_value.tv_sec = 1;
	ts.it_value.tv_nsec = 0;
	ts.it_interval.tv_sec = 0;
	ts.it_interval.tv_nsec = NANOSEC / 2;

	if (timer_settime(tid, TIMER_RELTIME, &ts, NULL) == -1) {
		(void) fprintf(stderr, "%s: timer_settime() failed: %s\n",
		    cmd, strerror(errno));
		exit(EXIT_FAILURE);
	}

	do {
		(void) sigwait(&set, &sig);
	} while(sig != SIGUSR1);

	/*NOTREACHED*/
	return (0);
}
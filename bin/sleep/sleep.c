
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
static char const copyright[] =
"@(#) Copyright (c) 1988, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)sleep.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void usage(void);

static volatile sig_atomic_t report_requested;
static void
report_request(int signo __unused)
{

	report_requested = 1;
}

int
main(int argc, char *argv[])
{
	struct timespec time_to_sleep;
	double d;
	time_t original;
	char buf[2];

	if (argc != 2)
		usage();

	if (sscanf(argv[1], "%lf%1s", &d, buf) != 1)
		usage();
	if (d > INT_MAX)
		usage();
	if (d <= 0)
		return (0);
	original = time_to_sleep.tv_sec = (time_t)d;
	time_to_sleep.tv_nsec = 1e9 * (d - time_to_sleep.tv_sec);

	signal(SIGINFO, report_request);
	while (nanosleep(&time_to_sleep, &time_to_sleep) != 0) {
		if (report_requested) {
			/* Reporting does not bother with nanoseconds. */
			warnx("about %d second(s) left out of the original %d",
			    (int)time_to_sleep.tv_sec, (int)original);
			report_requested = 0;
		} else
			break;
	}
	return (0);
}

static void
usage(void)
{

	fprintf(stderr, "usage: sleep seconds\n");
	exit(1);
}
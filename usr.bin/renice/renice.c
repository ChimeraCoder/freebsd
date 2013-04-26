
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
"@(#) Copyright (c) 1983, 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)renice.c	8.1 (Berkeley) 6/9/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int	donice(int, int, int, int);
static int	getnum(const char *, const char *, int *);
static void	usage(void);

/*
 * Change the priority (nice) of processes
 * or groups of processes which are already
 * running.
 */
int
main(int argc, char *argv[])
{
	struct passwd *pwd;
	int errs, incr, prio, which, who;

	errs = 0;
	incr = 0;
	which = PRIO_PROCESS;
	who = 0;
	argc--, argv++;
	if (argc < 2)
		usage();
	if (strcmp(*argv, "-n") == 0) {
		incr = 1;
		argc--, argv++;
		if (argc < 2)
			usage();
	}
	if (getnum("priority", *argv, &prio))
		return (1);
	argc--, argv++;
	for (; argc > 0; argc--, argv++) {
		if (strcmp(*argv, "-g") == 0) {
			which = PRIO_PGRP;
			continue;
		}
		if (strcmp(*argv, "-u") == 0) {
			which = PRIO_USER;
			continue;
		}
		if (strcmp(*argv, "-p") == 0) {
			which = PRIO_PROCESS;
			continue;
		}
		if (which == PRIO_USER) {
			if ((pwd = getpwnam(*argv)) != NULL)
				who = pwd->pw_uid;
			else if (getnum("uid", *argv, &who)) {
				errs++;
				continue;
			} else if (who < 0) {
				warnx("%s: bad value", *argv);
				errs++;
				continue;
			}
		} else {
			if (getnum("pid", *argv, &who)) {
				errs++;
				continue;
			}
			if (who < 0) {
				warnx("%s: bad value", *argv);
				errs++;
				continue;
			}
		}
		errs += donice(which, who, prio, incr);
	}
	exit(errs != 0);
}

static int
donice(int which, int who, int prio, int incr)
{
	int oldprio;

	errno = 0;
	oldprio = getpriority(which, who);
	if (oldprio == -1 && errno) {
		warn("%d: getpriority", who);
		return (1);
	}
	if (incr)
		prio = oldprio + prio;
	if (prio > PRIO_MAX)
		prio = PRIO_MAX;
	if (prio < PRIO_MIN)
		prio = PRIO_MIN;
	if (setpriority(which, who, prio) < 0) {
		warn("%d: setpriority", who);
		return (1);
	}
	fprintf(stderr, "%d: old priority %d, new priority %d\n", who,
	    oldprio, prio);
	return (0);
}

static int
getnum(const char *com, const char *str, int *val)
{
	long v;
	char *ep;

	errno = 0;
	v = strtol(str, &ep, 10);
	if (v < INT_MIN || v > INT_MAX || errno == ERANGE) {
		warnx("%s argument %s is out of range.", com, str);
		return (1);
	}
	if (ep == str || *ep != '\0' || errno != 0) {
		warnx("Bad %s argument: %s.", com, str);
		return (1);
	}

	*val = (int)v;
	return (0);
}

static void
usage(void)
{
	fprintf(stderr, "%s\n%s\n",
"usage: renice priority [[-p] pid ...] [[-g] pgrp ...] [[-u] user ...]",
"       renice -n increment [[-p] pid ...] [[-g] pgrp ...] [[-u] user ...]");
	exit(1);
}
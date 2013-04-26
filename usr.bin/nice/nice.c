
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
"@(#) Copyright (c) 1989, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)nice.c	8.2 (Berkeley) 4/16/94";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define	DEFNICE	10

void usage(void);

int
main(int argc, char *argv[])
{
	long niceness = DEFNICE;
	int ch;
	char *ep;

	/* Obsolescent syntax: -number, --number */
	if (argc >= 2 && argv[1][0] == '-' && (argv[1][1] == '-' ||
	    isdigit((unsigned char)argv[1][1])) && strcmp(argv[1], "--") != 0)
		if (asprintf(&argv[1], "-n%s", argv[1] + 1) < 0)
			err(1, "asprintf");

	while ((ch = getopt(argc, argv, "n:")) != -1) {
		switch (ch) {
		case 'n':
			errno = 0;
			niceness = strtol(optarg, &ep, 10);
			if (ep == optarg || *ep != '\0' || errno ||
			    niceness < INT_MIN || niceness > INT_MAX)
				errx(1, "%s: invalid nice value", optarg);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();

	errno = 0;
	niceness += getpriority(PRIO_PROCESS, 0);
	if (errno)
		warn("getpriority");
	else if (setpriority(PRIO_PROCESS, 0, (int)niceness))
		warn("setpriority");
	execvp(*argv, argv);
	err(errno == ENOENT ? 127 : 126, "%s", *argv);
}

void
usage(void)
{

	(void)fprintf(stderr,
	    "usage: nice [-n increment] utility [argument ...]\n");
	exit(1);
}

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
"@(#) Copyright (c) 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char const sccsid[] = "From: @(#)hostname.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(void);

int
main(int argc, char *argv[])
{
	int ch;
	char domainname[MAXHOSTNAMELEN];

	while ((ch = getopt(argc, argv, "")) != -1)
		switch (ch) {
		case '?':
			/* fall through */
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc > 1)
		usage();

	if (*argv) {
		if (setdomainname(*argv, (int)strlen(*argv)))
			err(1, "setdomainname");
	} else {
		if (getdomainname(domainname, (int)sizeof(domainname)))
			err(1, "getdomainname");
		(void)printf("%s\n", domainname);
	}
	exit(0);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: domainname [ypdomain]\n");
	exit(1);
}
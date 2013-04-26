
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
"@(#) Copyright (c) 1987, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)mesg.c	8.2 (Berkeley) 1/21/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(void);

int
main(int argc, char *argv[])
{
	struct stat sb;
	char *tty;
	int ch;

	while ((ch = getopt(argc, argv, "")) != -1)
		switch (ch) {
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if ((tty = ttyname(STDIN_FILENO)) == NULL &&
	    (tty = ttyname(STDOUT_FILENO)) == NULL &&
	    (tty = ttyname(STDERR_FILENO)) == NULL)
		err(2, "ttyname");
	if (stat(tty, &sb) < 0)
		err(2, "%s", tty);

	if (*argv == NULL) {
		if (sb.st_mode & S_IWGRP) {
			(void)puts("is y");
			exit(0);
		}
		(void)puts("is n");
		exit(1);
	}

	switch (*argv[0]) {
	case 'y':
		if (chmod(tty, sb.st_mode | S_IWGRP) < 0)
			err(2, "%s", tty);
		exit(0);
	case 'n':
		if (chmod(tty, sb.st_mode & ~S_IWGRP) < 0)
			err(2, "%s", tty);
		exit(1);
	}

	usage();
	return(0);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: mesg [n | y]\n");
	exit(2);
}
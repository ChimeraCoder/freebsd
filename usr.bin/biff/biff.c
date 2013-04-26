
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
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#if 0
#ifndef lint
static char sccsid[] = "@(#)biff.c	8.1 (Berkeley) 6/6/93";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(void);

int
main(int argc, char *argv[])
{
	struct stat sb;
	int ch;
	char *name;


	while ((ch = getopt(argc, argv, "")) != -1)
		switch(ch) {
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if ((name = ttyname(STDIN_FILENO)) == NULL &&
	    (name = ttyname(STDOUT_FILENO)) == NULL &&
	    (name = ttyname(STDERR_FILENO)) == NULL)
		err(2, "unknown tty");

	if (stat(name, &sb))
		err(2, "stat");

	if (*argv == NULL) {
		(void)printf("is %s\n",
		    sb.st_mode & S_IXUSR ? "y" :
		    sb.st_mode & S_IXGRP ? "b" : "n");
		return (sb.st_mode & (S_IXUSR | S_IXGRP) ? 0 : 1);

	}

	switch (argv[0][0]) {
	case 'n':
		if (chmod(name, sb.st_mode & ~(S_IXUSR | S_IXGRP)) < 0)
			err(2, "%s", name);
		break;
	case 'y':
		if (chmod(name, (sb.st_mode & ~(S_IXUSR | S_IXGRP)) | S_IXUSR)
		    < 0)
			err(2, "%s", name);
		break;
	case 'b':
		if (chmod(name, (sb.st_mode & ~(S_IXUSR | S_IXGRP)) | S_IXGRP)
		    < 0)
			err(2, "%s", name);
		break;
	default:
		usage();
	}
	return (sb.st_mode & (S_IXUSR | S_IXGRP) ? 0 : 1);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: biff [n | y | b]\n");
	exit(2);
}
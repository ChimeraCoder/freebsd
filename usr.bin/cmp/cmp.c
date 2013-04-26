
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
"@(#) Copyright (c) 1987, 1990, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#if 0
#ifndef lint
static char sccsid[] = "@(#)cmp.c	8.3 (Berkeley) 4/2/94";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

int	lflag, sflag, xflag, zflag;

static void usage(void);

int
main(int argc, char *argv[])
{
	struct stat sb1, sb2;
	off_t skip1, skip2;
	int ch, fd1, fd2, oflag, special;
	const char *file1, *file2;

	oflag = O_RDONLY;
	while ((ch = getopt(argc, argv, "hlsxz")) != -1)
		switch (ch) {
		case 'h':		/* Don't follow symlinks */
			oflag |= O_NOFOLLOW;
			break;
		case 'l':		/* print all differences */
			lflag = 1;
			break;
		case 's':		/* silent run */
			sflag = 1;
			zflag = 1;
			break;
		case 'x':		/* hex output */
			lflag = 1;
			xflag = 1;
			break;
		case 'z':		/* compare size first */
			zflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argv += optind;
	argc -= optind;

	if (lflag && sflag)
		errx(ERR_EXIT, "specifying -s with -l or -x is not permitted");

	if (argc < 2 || argc > 4)
		usage();

	/* Backward compatibility -- handle "-" meaning stdin. */
	special = 0;
	if (strcmp(file1 = argv[0], "-") == 0) {
		special = 1;
		fd1 = 0;
		file1 = "stdin";
	}
	else if ((fd1 = open(file1, oflag, 0)) < 0 && errno != EMLINK) {
		if (!sflag)
			err(ERR_EXIT, "%s", file1);
		else
			exit(ERR_EXIT);
	}
	if (strcmp(file2 = argv[1], "-") == 0) {
		if (special)
			errx(ERR_EXIT,
				"standard input may only be specified once");
		special = 1;
		fd2 = 0;
		file2 = "stdin";
	}
	else if ((fd2 = open(file2, oflag, 0)) < 0 && errno != EMLINK) {
		if (!sflag)
			err(ERR_EXIT, "%s", file2);
		else
			exit(ERR_EXIT);
	}

	skip1 = argc > 2 ? strtol(argv[2], NULL, 0) : 0;
	skip2 = argc == 4 ? strtol(argv[3], NULL, 0) : 0;

	if (fd1 == -1) {
		if (fd2 == -1) {
			c_link(file1, skip1, file2, skip2);
			exit(0);
		} else if (!sflag)
			errx(ERR_EXIT, "%s: Not a symbolic link", file2);
		else
			exit(ERR_EXIT);
	} else if (fd2 == -1) {
		if (!sflag)
			errx(ERR_EXIT, "%s: Not a symbolic link", file1);
		else
			exit(ERR_EXIT);
	}

	if (!special) {
		if (fstat(fd1, &sb1)) {
			if (!sflag)
				err(ERR_EXIT, "%s", file1);
			else
				exit(ERR_EXIT);
		}
		if (!S_ISREG(sb1.st_mode))
			special = 1;
		else {
			if (fstat(fd2, &sb2)) {
				if (!sflag)
					err(ERR_EXIT, "%s", file2);
				else
					exit(ERR_EXIT);
			}
			if (!S_ISREG(sb2.st_mode))
				special = 1;
		}
	}

	if (special)
		c_special(fd1, file1, skip1, fd2, file2, skip2);
	else {
		if (zflag && sb1.st_size != sb2.st_size) {
			if (!sflag)
				(void) printf("%s %s differ: size\n",
				    file1, file2);
			exit(DIFF_EXIT);
		}
		c_regular(fd1, file1, skip1, sb1.st_size,
		    fd2, file2, skip2, sb2.st_size);
	}
	exit(0);
}

static void
usage(void)
{

	(void)fprintf(stderr,
	    "usage: cmp [-l | -s | -x] [-hz] file1 file2 [skip1 [skip2]]\n");
	exit(ERR_EXIT);
}
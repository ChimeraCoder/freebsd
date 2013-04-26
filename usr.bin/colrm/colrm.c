
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
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#if 0
#ifndef lint
static char sccsid[] = "@(#)colrm.c	8.2 (Berkeley) 5/4/95";
#endif
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

#define	TAB	8

void check(FILE *);
static void usage(void);

int
main(int argc, char *argv[])
{
	u_long column, start, stop;
	int ch, width;
	char *p;

	setlocale(LC_ALL, "");

	while ((ch = getopt(argc, argv, "")) != -1)
		switch(ch) {
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	start = stop = 0;
	switch(argc) {
	case 2:
		stop = strtol(argv[1], &p, 10);
		if (stop <= 0 || *p)
			errx(1, "illegal column -- %s", argv[1]);
		/* FALLTHROUGH */
	case 1:
		start = strtol(argv[0], &p, 10);
		if (start <= 0 || *p)
			errx(1, "illegal column -- %s", argv[0]);
		break;
	case 0:
		break;
	default:
		usage();
	}

	if (stop && start > stop)
		errx(1, "illegal start and stop columns");

	for (column = 0;;) {
		switch (ch = getwchar()) {
		case WEOF:
			check(stdin);
			break;
		case '\b':
			if (column)
				--column;
			break;
		case '\n':
			column = 0;
			break;
		case '\t':
			column = (column + TAB) & ~(TAB - 1);
			break;
		default:
			if ((width = wcwidth(ch)) > 0)
				column += width;
			break;
		}

		if ((!start || column < start || (stop && column > stop)) &&
		    putwchar(ch) == WEOF)
			check(stdout);
	}
}

void
check(FILE *stream)
{
	if (feof(stream))
		exit(0);
	if (ferror(stream))
		err(1, "%s", stream == stdin ? "stdin" : "stdout");
}

void
usage(void)
{
	(void)fprintf(stderr, "usage: colrm [start [stop]]\n");
	exit(1);
}
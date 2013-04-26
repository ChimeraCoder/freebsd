
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1980, 1988, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#ifndef lint
static const char sccsid[] = "@(#)what.c	8.1 (Berkeley) 6/6/93";
#endif

#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(void);
static bool search(bool, bool, FILE *);

int
main(int argc, char *argv[])
{
	const char *file;
	FILE *in;
	bool found, qflag, sflag;
	int c;

	qflag = sflag = false;

	while ((c = getopt(argc, argv, "qs")) != -1) {
		switch (c) {
		case 'q':
			qflag = true;
			break;
		case 's':
			sflag = true;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	found = false;

	if (argc == 0) {
		if (search(sflag, qflag, stdin))
			found = true;
	} else {
		while (argc--) {
			file = *argv++;
			in = fopen(file, "r");
			if (in == NULL) {
				if (!qflag)
					warn("%s", file);
				continue;
			}
			if (!qflag)
				printf("%s:\n", file);
			if (search(sflag, qflag, in))
				found = true;
			fclose(in);
		}
	}
	exit(found ? 0 : 1);
}

static void
usage(void)
{
	fprintf(stderr, "usage: what [-qs] [file ...]\n");
	exit(1);
}

bool
search(bool one, bool quiet, FILE *in)
{
	bool found;
	int c;

	found = false;

	while ((c = getc(in)) != EOF) {
loop:		if (c != '@')
			continue;
		if ((c = getc(in)) != '(')
			goto loop;
		if ((c = getc(in)) != '#')
			goto loop;
		if ((c = getc(in)) != ')')
			goto loop;
		if (!quiet)
			putchar('\t');
		while ((c = getc(in)) != EOF && c && c != '"' &&
		    c != '>' && c != '\\' && c != '\n')
			putchar(c);
		putchar('\n');
		found = true;
		if (one)
			break;
	}
	return (found);
}
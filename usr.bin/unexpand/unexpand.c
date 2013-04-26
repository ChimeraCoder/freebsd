
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
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#ifndef lint
static const char sccsid[] = "@(#)unexpand.c	8.1 (Berkeley) 6/6/93";
#endif

/*
 * unexpand - put tabs into a file replacing blanks
 */
#include <ctype.h>
#include <err.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

static int	all;
static int	nstops;
static int	tabstops[100];

static void getstops(const char *);
static void usage(void);
static int tabify(const char *);

int
main(int argc, char *argv[])
{
	int ch, failed;
	char *filename;

	setlocale(LC_CTYPE, "");

	nstops = 1;
	tabstops[0] = 8;
	while ((ch = getopt(argc, argv, "at:")) != -1) {
		switch (ch) {
		case 'a':	/* Un-expand all spaces, not just leading. */
			all = 1;
			break;
		case 't':	/* Specify tab list, implies -a. */
			getstops(optarg);
			all = 1;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	}
	argc -= optind;
	argv += optind;

	failed = 0;
	if (argc == 0)
		failed |= tabify("stdin");
	else {
		while ((filename = *argv++) != NULL) {
			if (freopen(filename, "r", stdin) == NULL) {
				warn("%s", filename);
				failed = 1;
			} else
				failed |= tabify(filename);
		}
	}
	exit(failed != 0);
}

static void
usage(void)
{
	fprintf(stderr, "usage: unexpand [-a | -t tablist] [file ...]\n");
	exit(1);
}

static int
tabify(const char *curfile)
{
	int dcol, doneline, limit, n, ocol, width;
	wint_t ch;

	limit = nstops == 1 ? INT_MAX : tabstops[nstops - 1] - 1;

	doneline = ocol = dcol = 0;
	while ((ch = getwchar()) != WEOF) {
		if (ch == ' ' && !doneline) {
			if (++dcol >= limit)
				doneline = 1;
			continue;
		} else if (ch == '\t') {
			if (nstops == 1) {
				dcol = (1 + dcol / tabstops[0]) *
				    tabstops[0];
				continue;
			} else {
				for (n = 0; tabstops[n] - 1 < dcol &&
				    n < nstops; n++)
					;
				if (n < nstops - 1 && tabstops[n] - 1 < limit) {
					dcol = tabstops[n];
					continue;
				}
				doneline = 1;
			}
		}

		/* Output maximal number of tabs. */
		if (nstops == 1) {
			while (((ocol + tabstops[0]) / tabstops[0])
			    <= (dcol / tabstops[0])) {
				if (dcol - ocol < 2)
					break;
				putwchar('\t');
				ocol = (1 + ocol / tabstops[0]) *
				    tabstops[0];
			}
		} else {
			for (n = 0; tabstops[n] - 1 < ocol && n < nstops; n++)
				;
			while (ocol < dcol && n < nstops && ocol < limit) {
				putwchar('\t');
				ocol = tabstops[n++];
			}
		}

		/* Then spaces. */
		while (ocol < dcol && ocol < limit) {
			putwchar(' ');
			ocol++;
		}

		if (ch == '\b') {
			putwchar('\b');
			if (ocol > 0)
				ocol--, dcol--;
		} else if (ch == '\n') {
			putwchar('\n');
			doneline = ocol = dcol = 0;
			continue;
		} else if (ch != ' ' || dcol > limit) {
			putwchar(ch);
			if ((width = wcwidth(ch)) > 0)
				ocol += width, dcol += width;
		}

		/*
		 * Only processing leading blanks or we've gone past the
		 * last tab stop. Emit remainder of this line unchanged.
		 */
		if (!all || dcol >= limit) {
			while ((ch = getwchar()) != '\n' && ch != WEOF)
				putwchar(ch);
			if (ch == '\n')
				putwchar('\n');
			doneline = ocol = dcol = 0;
		}
	}
	if (ferror(stdin)) {
		warn("%s", curfile);
		return (1);
	}
	return (0);
}

static void
getstops(const char *cp)
{
	int i;

	nstops = 0;
	for (;;) {
		i = 0;
		while (*cp >= '0' && *cp <= '9')
			i = i * 10 + *cp++ - '0';
		if (i <= 0)
			errx(1, "bad tab stop spec");
		if (nstops > 0 && i <= tabstops[nstops-1])
			errx(1, "bad tab stop spec");
		if (nstops == sizeof(tabstops) / sizeof(*tabstops))
			errx(1, "too many tab stops");
		tabstops[nstops++] = i;
		if (*cp == 0)
			break;
		if (*cp != ',' && !isblank((unsigned char)*cp))
			errx(1, "bad tab stop spec");
		cp++;
	}
}
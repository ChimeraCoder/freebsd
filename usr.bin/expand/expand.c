
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
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)expand.c	8.1 (Berkeley) 6/9/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <ctype.h>
#include <err.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

/*
 * expand - expand tabs to equivalent spaces
 */
static int	nstops;
static int	tabstops[100];

static void getstops(char *);
static void usage(void);

int
main(int argc, char *argv[])
{
	const char *curfile;
	wint_t wc;
	int c, column;
	int n;
	int rval;
	int width;

	setlocale(LC_CTYPE, "");

	/* handle obsolete syntax */
	while (argc > 1 && argv[1][0] == '-' &&
	    isdigit((unsigned char)argv[1][1])) {
		getstops(&argv[1][1]);
		argc--; argv++;
	}

	while ((c = getopt (argc, argv, "t:")) != -1) {
		switch (c) {
		case 't':
			getstops(optarg);
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	rval = 0;
	do {
		if (argc > 0) {
			if (freopen(argv[0], "r", stdin) == NULL) {
				warn("%s", argv[0]);
				rval = 1;
				argc--, argv++;
				continue;
			}
			curfile = argv[0];
			argc--, argv++;
		} else
			curfile = "stdin";
		column = 0;
		while ((wc = getwchar()) != WEOF) {
			switch (wc) {
			case '\t':
				if (nstops == 0) {
					do {
						putwchar(' ');
						column++;
					} while (column & 07);
					continue;
				}
				if (nstops == 1) {
					do {
						putwchar(' ');
						column++;
					} while (((column - 1) % tabstops[0]) != (tabstops[0] - 1));
					continue;
				}
				for (n = 0; n < nstops; n++)
					if (tabstops[n] > column)
						break;
				if (n == nstops) {
					putwchar(' ');
					column++;
					continue;
				}
				while (column < tabstops[n]) {
					putwchar(' ');
					column++;
				}
				continue;

			case '\b':
				if (column)
					column--;
				putwchar('\b');
				continue;

			default:
				putwchar(wc);
				if ((width = wcwidth(wc)) > 0)
					column += width;
				continue;

			case '\n':
				putwchar(wc);
				column = 0;
				continue;
			}
		}
		if (ferror(stdin)) {
			warn("%s", curfile);
			rval = 1;
		}
	} while (argc > 0);
	exit(rval);
}

static void
getstops(char *cp)
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

static void
usage(void)
{
	(void)fprintf (stderr, "usage: expand [-t tablist] [file ...]\n");
	exit(1);
}
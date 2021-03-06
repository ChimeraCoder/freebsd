
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
"@(#) Copyright (c) 1988, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)number.c	8.3 (Berkeley) 5/4/95";
#endif
static const char rcsid[] =
 "$FreeBSD$";
#endif /* not lint */

#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	MAXNUM		65		/* Biggest number we handle. */

static const char	*name1[] = {
	"",		"one",		"two",		"three",
	"four",		"five",		"six",		"seven",
	"eight",	"nine",		"ten",		"eleven",
	"twelve",	"thirteen",	"fourteen",	"fifteen",
	"sixteen",	"seventeen",	"eighteen",	"nineteen",
},
		*name2[] = {
	"",		"ten",		"twenty",	"thirty",
	"forty",	"fifty",	"sixty",	"seventy",
	"eighty",	"ninety",
},
		*name3[] = {
	"hundred",	"thousand",	"million",	"billion",
	"trillion",	"quadrillion",	"quintillion",	"sextillion",
	"septillion",	"octillion",	"nonillion",	"decillion",
	"undecillion",	"duodecillion",	"tredecillion",	"quattuordecillion",
	"quindecillion",		"sexdecillion",
	"septendecillion",		"octodecillion",
	"novemdecillion",		"vigintillion",
};

static void	convert(char *);
static int	number(char *, int);
static void	pfract(int);
static int	unit(int, char *);
static void	usage(void);

static int lflag;

int
main(int argc, char *argv[])
{
	int ch, first;
	char line[256];

	lflag = 0;
	while ((ch = getopt(argc, argv, "l")) != -1)
		switch (ch) {
		case 'l':
			lflag = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (*argv == NULL)
		for (first = 1;
		    fgets(line, sizeof(line), stdin) != NULL; first = 0) {
			if (strchr(line, '\n') == NULL)
				errx(1, "line too long.");
			if (!first)
				(void)printf("...\n");
			convert(line);
		}
	else
		for (first = 1; *argv != NULL; first = 0, ++argv) {
			if (!first)
				(void)printf("...\n");
			convert(*argv);
		}
	exit(0);
}

static void
convert(char *line)
{
	int flen, len, rval;
	char *p, *fraction;

	flen = 0;
	fraction = NULL;
	for (p = line; *p != '\0' && *p != '\n'; ++p) {
		if (isblank(*p)) {
			if (p == line) {
				++line;
				continue;
			}
			goto badnum;
		}
		if (isdigit(*p))
			continue;
		switch (*p) {
		case '.':
			if (fraction != NULL)
				goto badnum;
			fraction = p + 1;
			*p = '\0';
			break;
		case '-':
			if (p == line)
				break;
			/* FALLTHROUGH */
		default:
badnum:			errx(1, "illegal number: %s", line);
			break;
		}
	}
	*p = '\0';

	if ((len = strlen(line)) > MAXNUM ||
	    (fraction != NULL && ((flen = strlen(fraction)) > MAXNUM)))
		errx(1, "number too large, max %d digits.", MAXNUM);

	if (*line == '-') {
		(void)printf("minus%s", lflag ? " " : "\n");
		++line;
		--len;
	}

	rval = len > 0 ? unit(len, line) : 0;
	if (fraction != NULL && flen != 0)
		for (p = fraction; *p != '\0'; ++p)
			if (*p != '0') {
				if (rval)
					(void)printf("%sand%s",
					    lflag ? " " : "",
					    lflag ? " " : "\n");
				if (unit(flen, fraction)) {
					if (lflag)
						(void)printf(" ");
					pfract(flen);
					rval = 1;
				}
				break;
			}
	if (!rval)
		(void)printf("zero%s", lflag ? "" : ".\n");
	if (lflag)
		(void)printf("\n");
}

static int
unit(int len, char *p)
{
	int off, rval;

	rval = 0;
	if (len > 3) {
		if (len % 3) {
			off = len % 3;
			len -= off;
			if (number(p, off)) {
				rval = 1;
				(void)printf(" %s%s",
				    name3[len / 3], lflag ? " " : ".\n");
			}
			p += off;
		}
		for (; len > 3; p += 3) {
			len -= 3;
			if (number(p, 3)) {
				rval = 1;
				(void)printf(" %s%s",
				    name3[len / 3], lflag ? " " : ".\n");
			}
		}
	}
	if (number(p, len)) {
		if (!lflag)
			(void)printf(".\n");
		rval = 1;
	}
	return (rval);
}

static int
number(char *p, int len)
{
	int val, rval;

	rval = 0;
	switch (len) {
	case 3:
		if (*p != '0') {
			rval = 1;
			(void)printf("%s hundred", name1[*p - '0']);
		}
		++p;
		/* FALLTHROUGH */
	case 2:
		val = (p[1] - '0') + (p[0] - '0') * 10;
		if (val) {
			if (rval)
				(void)printf(" ");
			if (val < 20)
				(void)printf("%s", name1[val]);
			else {
				(void)printf("%s", name2[val / 10]);
				if (val % 10)
					(void)printf("-%s", name1[val % 10]);
			}
			rval = 1;
		}
		break;
	case 1:
		if (*p != '0') {
			rval = 1;
			(void)printf("%s", name1[*p - '0']);
		}
	}
	return (rval);
}

static void
pfract(int len)
{
	static char const * const pref[] = { "", "ten-", "hundred-" };

	switch(len) {
	case 1:
		(void)printf("tenths.\n");
		break;
	case 2:
		(void)printf("hundredths.\n");
		break;
	default:
		(void)printf("%s%sths.\n", pref[len % 3], name3[len / 3]);
		break;
	}
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: number [-l] [# ...]\n");
	exit(1);
}
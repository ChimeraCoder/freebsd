
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
"@(#) Copyright (c) 1991, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif

#if 0
#ifndef lint
static char sccsid[] = "@(#)basename.c	8.4 (Berkeley) 5/4/95";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <err.h>
#include <libgen.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

void stripsuffix(char *, const char *, size_t);
void usage(void);

int
main(int argc, char **argv)
{
	char *p, *suffix;
	size_t suffixlen;
	int aflag, ch;

	setlocale(LC_ALL, "");

	aflag = 0;
	suffix = NULL;
	suffixlen = 0;

	while ((ch = getopt(argc, argv, "as:")) != -1)
		switch(ch) {
		case 'a':
			aflag = 1;
			break;
		case 's':
			suffix = optarg;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc < 1)
		usage();

	if (!*argv[0]) {
		printf("\n");
		exit(0);
	}
	if ((p = basename(argv[0])) == NULL)
		err(1, "%s", argv[0]);
	if ((suffix == NULL && !aflag) && argc == 2) {
		suffix = argv[1];
		argc--;
	}
	if (suffix != NULL)
		suffixlen = strlen(suffix);
	while (argc--) {
		if ((p = basename(*argv)) == NULL)
			err(1, "%s", argv[0]);
		stripsuffix(p, suffix, suffixlen);
		argv++;
		(void)printf("%s\n", p);
	}
	exit(0);
}

void
stripsuffix(char *p, const char *suffix, size_t suffixlen)
{
	char *q, *r;
	mbstate_t mbs;
	size_t n;

	if (suffixlen && (q = strchr(p, '\0') - suffixlen) > p &&
	    strcmp(suffix, q) == 0) {
		/* Ensure that the match occurred on a character boundary. */
		memset(&mbs, 0, sizeof(mbs));
		for (r = p; r < q; r += n) {
			n = mbrlen(r, MB_LEN_MAX, &mbs);
			if (n == (size_t)-1 || n == (size_t)-2) {
				memset(&mbs, 0, sizeof(mbs));
				n = 1;
			}
		}
		/* Chop off the suffix. */
		if (q == r)
			*q = '\0';
	}
}

void
usage(void)
{

	(void)fprintf(stderr,
"usage: basename string [suffix]\n"
"       basename [-a] [-s suffix] string [...]\n");
	exit(1);
}
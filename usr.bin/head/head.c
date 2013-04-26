
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
"@(#) Copyright (c) 1980, 1987, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)head.c	8.2 (Berkeley) 5/4/95";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * head - give the first few lines of a stream or of each of a set of files
 *
 * Bill Joy UCB August 24, 1977
 */

static void head(FILE *, int);
static void head_bytes(FILE *, off_t);
static void obsolete(char *[]);
static void usage(void);

int
main(int argc, char *argv[])
{
	int ch;
	FILE *fp;
	int first, linecnt = -1, eval = 0;
	off_t bytecnt = -1;
	char *ep;

	obsolete(argv);
	while ((ch = getopt(argc, argv, "n:c:")) != -1)
		switch(ch) {
		case 'c':
			bytecnt = strtoimax(optarg, &ep, 10);
			if (*ep || bytecnt <= 0)
				errx(1, "illegal byte count -- %s", optarg);
			break;
		case 'n':
			linecnt = strtol(optarg, &ep, 10);
			if (*ep || linecnt <= 0)
				errx(1, "illegal line count -- %s", optarg);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (linecnt != -1 && bytecnt != -1)
		errx(1, "can't combine line and byte counts");
	if (linecnt == -1 )
		linecnt = 10;
	if (*argv) {
		for (first = 1; *argv; ++argv) {
			if ((fp = fopen(*argv, "r")) == NULL) {
				warn("%s", *argv);
				eval = 1;
				continue;
			}
			if (argc > 1) {
				(void)printf("%s==> %s <==\n",
				    first ? "" : "\n", *argv);
				first = 0;
			}
			if (bytecnt == -1)
				head(fp, linecnt);
			else
				head_bytes(fp, bytecnt);
			(void)fclose(fp);
		}
	} else if (bytecnt == -1)
		head(stdin, linecnt);
	else
		head_bytes(stdin, bytecnt);

	exit(eval);
}

static void
head(FILE *fp, int cnt)
{
	char *cp;
	size_t error, readlen;

	while (cnt && (cp = fgetln(fp, &readlen)) != NULL) {
		error = fwrite(cp, sizeof(char), readlen, stdout);
		if (error != readlen)
			err(1, "stdout");
		cnt--;
	}
}

static void
head_bytes(FILE *fp, off_t cnt)
{
	char buf[4096];
	size_t readlen;

	while (cnt) {
		if ((uintmax_t)cnt < sizeof(buf))
			readlen = cnt;
		else
			readlen = sizeof(buf);
		readlen = fread(buf, sizeof(char), readlen, fp);
		if (readlen == 0)
			break;
		if (fwrite(buf, sizeof(char), readlen, stdout) != readlen)
			err(1, "stdout");
		cnt -= readlen;
	}
}

static void
obsolete(char *argv[])
{
	char *ap;

	while ((ap = *++argv)) {
		/* Return if "--" or not "-[0-9]*". */
		if (ap[0] != '-' || ap[1] == '-' || !isdigit(ap[1]))
			return;
		if ((ap = malloc(strlen(*argv) + 2)) == NULL)
			err(1, NULL);
		ap[0] = '-';
		ap[1] = 'n';
		(void)strcpy(ap + 2, *argv + 1);
		*argv = ap;
	}
}

static void
usage(void)
{

	(void)fprintf(stderr, "usage: head [-n lines | -c bytes] [file ...]\n");
	exit(1);
}
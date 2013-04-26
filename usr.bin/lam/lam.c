
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
"@(#) Copyright (c) 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)lam.c	8.1 (Berkeley) 6/6/93";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 *	lam - laminate files
 *	Author:  John Kunze, UCB
 */

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	MAXOFILES	20
#define	BIGBUFSIZ	5 * BUFSIZ

static struct openfile {	/* open file structure */
	FILE	*fp;		/* file pointer */
	short	eof;		/* eof flag */
	short	pad;		/* pad flag for missing columns */
	char	eol;		/* end of line character */
	const char *sepstring;	/* string to print before each line */
	const char *format;	/* printf(3) style string spec. */
}	input[MAXOFILES];

static int	morefiles;	/* set by getargs(), changed by gatherline() */
static int	nofinalnl;	/* normally append \n to each output line */
static char	line[BIGBUFSIZ];
static char	*linep;

static char    *gatherline(struct openfile *);
static void	getargs(char *[]);
static char    *pad(struct openfile *);
static void	usage(void);

int
main(int argc, char *argv[])
{
	struct	openfile *ip;

	if (argc == 1)
		usage();
	getargs(argv);
	if (!morefiles)
		usage();
	for (;;) {
		linep = line;
		for (ip = input; ip->fp != NULL; ip++)
			linep = gatherline(ip);
		if (!morefiles)
			exit(0);
		fputs(line, stdout);
		fputs(ip->sepstring, stdout);
		if (!nofinalnl)
			putchar('\n');
	}
}

static void
getargs(char *av[])
{
	struct	openfile *ip = input;
	char *p, *c;
	static char fmtbuf[BUFSIZ];
	char *fmtp = fmtbuf;
	int P, S, F, T;

	P = S = F = T = 0;		/* capitalized options */
	while ((p = *++av) != NULL) {
		if (*p != '-' || !p[1]) {
			if (++morefiles >= MAXOFILES)
				errx(1, "too many input files");
			if (*p == '-')
				ip->fp = stdin;
			else if ((ip->fp = fopen(p, "r")) == NULL) {
				err(1, "%s", p);
			}
			ip->pad = P;
			if (!ip->sepstring)
				ip->sepstring = (S ? (ip-1)->sepstring : "");
			if (!ip->format)
				ip->format = ((P || F) ? (ip-1)->format : "%s");
			if (!ip->eol)
				ip->eol = (T ? (ip-1)->eol : '\n');
			ip++;
			continue;
		}
		c = ++p;
		switch (tolower((unsigned char)*c)) {
		case 's':
			if (*++p || (p = *++av))
				ip->sepstring = p;
			else
				usage();
			S = (*c == 'S' ? 1 : 0);
			break;
		case 't':
			if (*++p || (p = *++av))
				ip->eol = *p;
			else
				usage();
			T = (*c == 'T' ? 1 : 0);
			nofinalnl = 1;
			break;
		case 'p':
			ip->pad = 1;
			P = (*c == 'P' ? 1 : 0);
			/* FALLTHROUGH */
		case 'f':
			F = (*c == 'F' ? 1 : 0);
			if (*++p || (p = *++av)) {
				fmtp += strlen(fmtp) + 1;
				if (fmtp >= fmtbuf + sizeof(fmtbuf))
					errx(1, "no more format space");
				/* restrict format string to only valid width formatters */
				if (strspn(p, "-.0123456789") != strlen(p))
					errx(1, "invalid format string `%s'", p);
				if (snprintf(fmtp, fmtbuf + sizeof(fmtbuf) - fmtp, "%%%ss", p)
				    >= fmtbuf + sizeof(fmtbuf) - fmtp)
					errx(1, "no more format space");
				ip->format = fmtp;
			}
			else
				usage();
			break;
		default:
			usage();
		}
	}
	ip->fp = NULL;
	if (!ip->sepstring)
		ip->sepstring = "";
}

static char *
pad(struct openfile *ip)
{
	char *lp = linep;

	strlcpy(lp, ip->sepstring, line + sizeof(line) - lp);
	lp += strlen(lp);
	if (ip->pad) {
		snprintf(lp, line + sizeof(line) - lp, ip->format, "");
		lp += strlen(lp);
	}
	return (lp);
}

static char *
gatherline(struct openfile *ip)
{
	char s[BUFSIZ];
	int c;
	char *p;
	char *lp = linep;
	char *end = s + sizeof(s) - 1;

	if (ip->eof)
		return (pad(ip));
	for (p = s; (c = fgetc(ip->fp)) != EOF && p < end; p++)
		if ((*p = c) == ip->eol)
			break;
	*p = '\0';
	if (c == EOF) {
		ip->eof = 1;
		if (ip->fp == stdin)
			fclose(stdin);
		morefiles--;
		return (pad(ip));
	}
	strlcpy(lp, ip->sepstring, line + sizeof(line) - lp);
	lp += strlen(lp);
	snprintf(lp, line + sizeof(line) - lp, ip->format, s);
	lp += strlen(lp);
	return (lp);
}

static void
usage(void)
{
	fprintf(stderr, "%s\n%s\n",
"usage: lam [ -f min.max ] [ -s sepstring ] [ -t c ] file ...",
"       lam [ -p min.max ] [ -s sepstring ] [ -t c ] file ...");
	exit(1);
}

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
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)paste.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
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

static wchar_t *delim;
static int delimcnt;

static int parallel(char **);
static int sequential(char **);
static int tr(wchar_t *);
static void usage(void);

static wchar_t tab[] = L"\t";

int
main(int argc, char *argv[])
{
	int ch, rval, seq;
	wchar_t *warg;
	const char *arg;
	size_t len;

	setlocale(LC_CTYPE, "");

	seq = 0;
	while ((ch = getopt(argc, argv, "d:s")) != -1)
		switch(ch) {
		case 'd':
			arg = optarg;
			len = mbsrtowcs(NULL, &arg, 0, NULL);
			if (len == (size_t)-1)
				err(1, "delimiters");
			warg = malloc((len + 1) * sizeof(*warg));
			if (warg == NULL)
				err(1, NULL);
			arg = optarg;
			len = mbsrtowcs(warg, &arg, len + 1, NULL);
			if (len == (size_t)-1)
				err(1, "delimiters");
			delimcnt = tr(delim = warg);
			break;
		case 's':
			seq = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (*argv == NULL)
		usage();
	if (!delim) {
		delimcnt = 1;
		delim = tab;
	}

	if (seq)
		rval = sequential(argv);
	else
		rval = parallel(argv);
	exit(rval);
}

typedef struct _list {
	struct _list *next;
	FILE *fp;
	int cnt;
	char *name;
} LIST;

static int
parallel(char **argv)
{
	LIST *lp;
	int cnt;
	wint_t ich;
	wchar_t ch;
	char *p;
	LIST *head, *tmp;
	int opencnt, output;

	for (cnt = 0, head = tmp = NULL; (p = *argv); ++argv, ++cnt) {
		if ((lp = malloc(sizeof(LIST))) == NULL)
			err(1, NULL);
		if (p[0] == '-' && !p[1])
			lp->fp = stdin;
		else if (!(lp->fp = fopen(p, "r")))
			err(1, "%s", p);
		lp->next = NULL;
		lp->cnt = cnt;
		lp->name = p;
		if (!head)
			head = tmp = lp;
		else {
			tmp->next = lp;
			tmp = lp;
		}
	}

	for (opencnt = cnt; opencnt;) {
		for (output = 0, lp = head; lp; lp = lp->next) {
			if (!lp->fp) {
				if (output && lp->cnt &&
				    (ch = delim[(lp->cnt - 1) % delimcnt]))
					putwchar(ch);
				continue;
			}
			if ((ich = getwc(lp->fp)) == WEOF) {
				if (!--opencnt)
					break;
				lp->fp = NULL;
				if (output && lp->cnt &&
				    (ch = delim[(lp->cnt - 1) % delimcnt]))
					putwchar(ch);
				continue;
			}
			/*
			 * make sure that we don't print any delimiters
			 * unless there's a non-empty file.
			 */
			if (!output) {
				output = 1;
				for (cnt = 0; cnt < lp->cnt; ++cnt)
					if ((ch = delim[cnt % delimcnt]))
						putwchar(ch);
			} else if ((ch = delim[(lp->cnt - 1) % delimcnt]))
				putwchar(ch);
			if (ich == '\n')
				continue;
			do {
				putwchar(ich);
			} while ((ich = getwc(lp->fp)) != WEOF && ich != '\n');
		}
		if (output)
			putwchar('\n');
	}

	return (0);
}

static int
sequential(char **argv)
{
	FILE *fp;
	int cnt, failed, needdelim;
	wint_t ch;
	char *p;

	failed = 0;
	for (; (p = *argv); ++argv) {
		if (p[0] == '-' && !p[1])
			fp = stdin;
		else if (!(fp = fopen(p, "r"))) {
			warn("%s", p);
			failed = 1;
			continue;
		}
		cnt = needdelim = 0;
		while ((ch = getwc(fp)) != WEOF) {
			if (needdelim) {
				needdelim = 0;
				if (delim[cnt] != '\0')
					putwchar(delim[cnt]);
				if (++cnt == delimcnt)
					cnt = 0;
			}
			if (ch != '\n')
				putwchar(ch);
			else
				needdelim = 1;
		}
		if (needdelim)
			putwchar('\n');
		if (fp != stdin)
			(void)fclose(fp);
	}

	return (failed != 0);
}

static int
tr(wchar_t *arg)
{
	int cnt;
	wchar_t ch, *p;

	for (p = arg, cnt = 0; (ch = *p++); ++arg, ++cnt)
		if (ch == '\\')
			switch(ch = *p++) {
			case 'n':
				*arg = '\n';
				break;
			case 't':
				*arg = '\t';
				break;
			case '0':
				*arg = '\0';
				break;
			default:
				*arg = ch;
				break;
		} else
			*arg = ch;

	if (!cnt)
		errx(1, "no delimiters specified");
	return(cnt);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: paste [-s] [-d delimiters] file ...\n");
	exit(1);
}
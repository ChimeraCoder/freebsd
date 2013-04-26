
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
"@(#) Copyright (c) 1980, 1987, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)users.c	8.1 (Berkeley) 6/6/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/param.h>
#include <sys/types.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmpx.h>

typedef char   namebuf[sizeof(((struct utmpx *)0)->ut_user) + 1];

int scmp(const void *, const void *);
static void usage(void);

int
main(int argc, char **argv)
{
	namebuf *names = NULL;
	int ncnt = 0;
	int nmax = 0;
	int cnt;
	struct utmpx *ut;
	int ch;

	while ((ch = getopt(argc, argv, "")) != -1)
		switch(ch) {
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	setutxent();
	while ((ut = getutxent()) != NULL) {
		if (ut->ut_type != USER_PROCESS)
			continue;
		if (ncnt >= nmax) {
			nmax += 32;
			names = realloc(names, sizeof(*names) * nmax);
			if (!names) {
				errx(1, "realloc");
				/* NOTREACHED */
			}
		}
		(void)strlcpy(names[ncnt], ut->ut_user, sizeof(*names));
		++ncnt;
	}
	endutxent();
	if (ncnt > 0) {
		qsort(names, ncnt, sizeof(namebuf), scmp);
		(void)printf("%s", names[0]);
		for (cnt = 1; cnt < ncnt; ++cnt)
			if (strcmp(names[cnt], names[cnt - 1]) != 0)
				(void)printf(" %s", names[cnt]);
		(void)printf("\n");
	}
	exit(0);
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage: users\n");
	exit(1);
}
	
int
scmp(const void *p, const void *q)
{

	return (strcmp(p, q));
}
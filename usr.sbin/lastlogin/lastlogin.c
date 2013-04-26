
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
#ifndef lint
__RCSID("$FreeBSD$");
__RCSID("$NetBSD: lastlogin.c,v 1.4 1998/02/03 04:45:35 perry Exp $");
#endif

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utmpx.h>

	int	main(int, char **);
static	void	output(struct utmpx *);
static	void	usage(void);
static int	utcmp_user(const void *, const void *);

static int	order = 1;
static const char *file = NULL;
static int	(*utcmp)(const void *, const void *) = utcmp_user;

static int
utcmp_user(const void *u1, const void *u2)
{

	return (order * strcmp(((const struct utmpx *)u1)->ut_user,
	    ((const struct utmpx *)u2)->ut_user));
}

static int
utcmp_time(const void *u1, const void *u2)
{
	time_t t1, t2;

	t1 = ((const struct utmpx *)u1)->ut_tv.tv_sec;
	t2 = ((const struct utmpx *)u2)->ut_tv.tv_sec;
	return (t1 < t2 ? order : t1 > t2 ? -order : 0);
}

int
main(int argc, char *argv[])
{
	int	ch, i, ulistsize;
	struct utmpx *u, *ulist;

	while ((ch = getopt(argc, argv, "f:rt")) != -1) {
		switch (ch) {
		case 'f':
			file = optarg;
			break;
		case 'r':
			order = -1;
			break;
		case 't':
			utcmp = utcmp_time;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0) {
		/* Process usernames given on the command line. */
		for (i = 0; i < argc; i++) {
			if (setutxdb(UTXDB_LASTLOGIN, file) != 0)
				err(1, "failed to open lastlog database");
			if ((u = getutxuser(argv[i])) == NULL) {
				warnx("user '%s' not found", argv[i]);
				continue;
			}
			output(u);
			endutxent();
		}
	} else {
		/* Read all lastlog entries, looking for active ones. */
		if (setutxdb(UTXDB_LASTLOGIN, file) != 0)
			err(1, "failed to open lastlog database");
		ulist = NULL;
		ulistsize = 0;
		while ((u = getutxent()) != NULL) {
			if (u->ut_type != USER_PROCESS)
				continue;
			if ((ulistsize % 16) == 0) {
				ulist = realloc(ulist,
				    (ulistsize + 16) * sizeof(struct utmpx));
				if (ulist == NULL)
					err(1, "malloc");
			}
			ulist[ulistsize++] = *u;
		}
		endutxent();

		qsort(ulist, ulistsize, sizeof(struct utmpx), utcmp);
		for (i = 0; i < ulistsize; i++)
			output(&ulist[i]);
	}

	exit(0);
}

/* Duplicate the output of last(1) */
static void
output(struct utmpx *u)
{
	time_t t = u->ut_tv.tv_sec;

	printf("%-10s %-8s %-22.22s %s",
		u->ut_user, u->ut_line, u->ut_host, ctime(&t));
}

static void
usage(void)
{
	fprintf(stderr, "usage: lastlogin [-f file] [-rt] [user ...]\n");
	exit(1);
}
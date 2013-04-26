
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

#if 0
#ifndef lint
static char const copyright[] =
"@(#) Copyright (c) 1991, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)pwd.c	8.3 (Berkeley) 4/1/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *getcwd_logical(void);
void usage(void);

int
main(int argc, char *argv[])
{
	int physical;
	int ch;
	char *p;

	physical = 1;
	while ((ch = getopt(argc, argv, "LP")) != -1)
		switch (ch) {
		case 'L':
			physical = 0;
			break;
		case 'P':
			physical = 1;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	/*
	 * If we're trying to find the logical current directory and that
	 * fails, behave as if -P was specified.
	 */
	if ((!physical && (p = getcwd_logical()) != NULL) ||
	    (p = getcwd(NULL, 0)) != NULL)
		printf("%s\n", p);
	else
		err(1, ".");

	exit(0);
}

void
usage(void)
{

	(void)fprintf(stderr, "usage: pwd [-L | -P]\n");
  	exit(1);
}

static char *
getcwd_logical(void)
{
	struct stat lg, phy;
	char *pwd;

	/*
	 * Check that $PWD is an absolute logical pathname referring to
	 * the current working directory.
	 */
	if ((pwd = getenv("PWD")) != NULL && *pwd == '/') {
		if (stat(pwd, &lg) == -1 || stat(".", &phy) == -1)
			return (NULL);
		if (lg.st_dev == phy.st_dev && lg.st_ino == phy.st_ino)
			return (pwd);
	}

	errno = ENOENT;
	return (NULL);
}
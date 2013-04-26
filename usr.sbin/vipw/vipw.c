
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
static const char copyright[] =
"@(#) Copyright (c) 1987, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)vipw.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libutil.h>		/* must be after pwd.h */

static void	usage(void);

int
main(int argc, char *argv[])
{
	const char *passwd_dir = NULL;
	int ch, pfd, tfd;
	char *line;
	size_t len;

	while ((ch = getopt(argc, argv, "d:")) != -1)
		switch (ch) {
		case 'd':
			passwd_dir = optarg;
			break;
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (argc != 0)
		usage();

	if (pw_init(passwd_dir, NULL) == -1)
		err(1, "pw_init()");
	if ((pfd = pw_lock()) == -1) {
		pw_fini();
		err(1, "pw_lock()");
	}
	if ((tfd = pw_tmp(pfd)) == -1) {
		pw_fini();
		err(1, "pw_tmp()");
	}
	(void)close(tfd);
	/* Force umask for partial writes made in the edit phase */
	(void)umask(077);

	for (;;) {
		switch (pw_edit(0)) {
		case -1:
			pw_fini();
			err(1, "pw_edit()");
		case 0:
			pw_fini();
			errx(0, "no changes made");
		default:
			break;
		}
		if (pw_mkdb(NULL) == 0) {
			pw_fini();
			errx(0, "password list updated");
		}
		printf("re-edit the password file? ");
		fflush(stdout);
		if ((line = fgetln(stdin, &len)) == NULL) {
			pw_fini();
			err(1, "fgetln()");
		}
		if (len > 0 && (*line == 'N' || *line == 'n'))
			break;
	}
	pw_fini();
	exit(0);
}

static void
usage(void)
{

	(void)fprintf(stderr, "usage: vipw [-d directory]\n");
	exit(1);
}

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
"@(#) Copyright (c) 1983, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#if 0
#ifndef lint
static char sccsid[] = "@(#)lprm.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif

#include "lp.cdefs.h"		/* A cross-platform version of <sys/cdefs.h> */
__FBSDID("$FreeBSD$");

/*
 * lprm - remove the current user's spool entry
 *
 * lprm [-] [[job #] [user] ...]
 *
 * Using information in the lock file, lprm will kill the
 * currently active daemon (if necessary), remove the associated files,
 * and startup a new daemon.  Priviledged users may remove anyone's spool
 * entries, otherwise one can only remove their own.
 */

#include <sys/param.h>

#include <syslog.h>
#include <dirent.h>
#include <err.h>
#include <pwd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lp.h"
#include "lp.local.h"

/*
 * Stuff for handling job specifications
 */
char	*person;		/* name of person doing lprm */
int	 requ[MAXREQUESTS];	/* job number of spool entries */
int	 requests;		/* # of spool requests */
char	*user[MAXUSERS];	/* users to process */
int	 users;			/* # of users in user array */
uid_t	 uid, euid;		/* real and effective user id's */

static char	luser[16];	/* buffer for person */

int		 main(int argc, char *_argv[]);
static void	 usage(void);

int
main(int argc, char *argv[])
{
	char *arg;
	const char *printer;
	struct passwd *p;
	static char root[] = "root";

	printer = NULL;
	uid = getuid();
	euid = geteuid();
	PRIV_END	/* be safe */
	progname = argv[0];
	gethostname(local_host, sizeof(local_host));
	openlog("lpd", 0, LOG_LPR);

	/*
	 * Bogus code later checks for string equality between 
	 * `person' and "root", so if we are root, better make sure
	 * that code will succeed.
	 */
	if (getuid() == 0) {
		person = root;
	} else if ((person = getlogin()) == NULL) {
		if ((p = getpwuid(getuid())) == NULL)
			fatal(0, "Who are you?");
		if (strlen(p->pw_name) >= sizeof(luser))
			fatal(0, "Your name is too long");
		strcpy(luser, p->pw_name);
		person = luser;
	}
	while (--argc) {
		if ((arg = *++argv)[0] == '-')
			switch (arg[1]) {
			case 'P':
				if (arg[2])
					printer = &arg[2];
				else if (argc > 1) {
					argc--;
					printer = *++argv;
				}
				break;
			case '\0':
				if (!users) {
					users = -1;
					break;
				}
			default:
				usage();
			}
		else {
			if (users < 0)
				usage();
			if (isdigit(arg[0])) {
				if (requests >= MAXREQUESTS)
					fatal(0, "Too many requests");
				requ[requests++] = atoi(arg);
			} else {
				if (users >= MAXUSERS)
					fatal(0, "Too many users");
				user[users++] = arg;
			}
		}
	}
	if (printer == NULL && (printer = getenv("PRINTER")) == NULL)
		printer = DEFLP;

	rmjob(printer);
	exit(0);
}

static void
usage(void)
{
	fprintf(stderr, "usage: lprm [-] [-Pprinter] [[job #] [user] ...]\n");
	exit(2);
}
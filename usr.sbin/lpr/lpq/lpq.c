
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
static char sccsid[] = "@(#)lpq.c	8.3 (Berkeley) 5/10/95";
#endif /* not lint */
#endif

#include "lp.cdefs.h"		/* A cross-platform version of <sys/cdefs.h> */
__FBSDID("$FreeBSD$");

/*
 * Spool Queue examination program
 *
 * lpq [-a] [-l] [-Pprinter] [user...] [job...]
 *
 * -a show all non-null queues on the local machine
 * -l long output
 * -P used to identify printer as per lpr/lprm
 */

#include <sys/param.h>

#include <ctype.h>
#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "lp.h"
#include "lp.local.h"
#include "pathnames.h"

int	 requ[MAXREQUESTS];	/* job number of spool entries */
int	 requests;		/* # of spool requests */
char	*user[MAXUSERS];	/* users to process */
int	 users;			/* # of users in user array */

uid_t	uid, euid;

static int	 ckqueue(const struct printer *_pp);
static void	 usage(void);
int 		 main(int _argc, char **_argv);

int
main(int argc, char **argv)
{
	int ch, aflag, lflag;
	const char *printer;
	struct printer myprinter, *pp = &myprinter;

	printer = NULL;
	euid = geteuid();
	uid = getuid();
	PRIV_END
	progname = *argv;
	if (gethostname(local_host, sizeof(local_host)))
		err(1, "gethostname");
	openlog("lpd", 0, LOG_LPR);

	aflag = lflag = 0;
	while ((ch = getopt(argc, argv, "alP:")) != -1)
		switch((char)ch) {
		case 'a':
			++aflag;
			break;
		case 'l':			/* long output */
			++lflag;
			break;
		case 'P':		/* printer name */
			printer = optarg;
			break;
		case '?':
		default:
			usage();
		}

	if (!aflag && printer == NULL && (printer = getenv("PRINTER")) == NULL)
		printer = DEFLP;

	for (argc -= optind, argv += optind; argc; --argc, ++argv)
		if (isdigit(argv[0][0])) {
			if (requests >= MAXREQUESTS)
				fatal(0, "too many requests");
			requ[requests++] = atoi(*argv);
		}
		else {
			if (users >= MAXUSERS)
				fatal(0, "too many users");
			user[users++] = *argv;
		}

	if (aflag) {
		int more, status;

		more = firstprinter(pp, &status);
		if (status)
			goto looperr;
		while (more) {
			if (ckqueue(pp) > 0) {
				printf("%s:\n", pp->printer);
				displayq(pp, lflag);
				printf("\n");
			}
			do {
				more = nextprinter(pp, &status);
looperr:
				switch (status) {
				case PCAPERR_TCOPEN:
					printf("warning: %s: unresolved "
					       "tc= reference(s) ",
					       pp->printer);
				case PCAPERR_SUCCESS:
					break;
				default:
					fatal(pp, "%s", pcaperr(status));
				}
			} while (more && status);
		}
	} else {
		int status;

		init_printer(pp);
		status = getprintcap(printer, pp);
		if (status < 0)
			fatal(pp, "%s", pcaperr(status));

		displayq(pp, lflag);
	}
	exit(0);
}

static int
ckqueue(const struct printer *pp)
{
	register struct dirent *d;
	DIR *dirp;
	char *spooldir;

	spooldir = pp->spool_dir;
	if ((dirp = opendir(spooldir)) == NULL)
		return (-1);
	while ((d = readdir(dirp)) != NULL) {
		if (d->d_name[0] != 'c' || d->d_name[1] != 'f')
			continue;	/* daemon control files only */
		closedir(dirp);
		return (1);		/* found something */
	}
	closedir(dirp);
	return (0);
}

static void
usage(void)
{
	fprintf(stderr,
	"usage: lpq [-a] [-l] [-Pprinter] [user ...] [job ...]\n");
	exit(1);
}
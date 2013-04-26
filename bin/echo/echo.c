
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
"@(#) Copyright (c) 1989, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)echo.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/uio.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Report an error and exit.
 * Use it instead of err(3) to avoid linking-in stdio.
 */
static __dead2 void
errexit(const char *prog, const char *reason)
{
	char *errstr = strerror(errno);
	write(STDERR_FILENO, prog, strlen(prog));
	write(STDERR_FILENO, ": ", 2);
	write(STDERR_FILENO, reason, strlen(reason));
	write(STDERR_FILENO, ": ", 2);
	write(STDERR_FILENO, errstr, strlen(errstr));
	write(STDERR_FILENO, "\n", 1);
	exit(1);
}
	
int
main(int argc, char *argv[])
{
	int nflag;	/* if not set, output a trailing newline. */
	int veclen;	/* number of writev arguments. */
	struct iovec *iov, *vp; /* Elements to write, current element. */
	char space[] = " ";
	char newline[] = "\n";
	char *progname = argv[0];

	/* This utility may NOT do getopt(3) option parsing. */
	if (*++argv && !strcmp(*argv, "-n")) {
		++argv;
		--argc;
		nflag = 1;
	} else
		nflag = 0;

	veclen = (argc >= 2) ? (argc - 2) * 2 + 1 : 0;

	if ((vp = iov = malloc((veclen + 1) * sizeof(struct iovec))) == NULL)
		errexit(progname, "malloc");

	while (argv[0] != NULL) {
		size_t len;
		
		len = strlen(argv[0]);

		/*
		 * If the next argument is NULL then this is this
		 * the last argument, therefore we need to check
		 * for a trailing \c.
		 */
		if (argv[1] == NULL) {
			/* is there room for a '\c' and is there one? */
			if (len >= 2 &&
			    argv[0][len - 2] == '\\' &&
			    argv[0][len - 1] == 'c') {
				/* chop it and set the no-newline flag. */
				len -= 2;
				nflag = 1;
			}
		}
		vp->iov_base = *argv;
		vp++->iov_len = len;
		if (*++argv) {
			vp->iov_base = space;
			vp++->iov_len = 1;
		}
	}
	if (!nflag) {
		veclen++;
		vp->iov_base = newline;
		vp++->iov_len = 1;
	}
	/* assert(veclen == (vp - iov)); */
	while (veclen) {
		int nwrite;

		nwrite = (veclen > IOV_MAX) ? IOV_MAX : veclen;
		if (writev(STDOUT_FILENO, iov, nwrite) == -1)
			errexit(progname, "write");
		iov += nwrite;
		veclen -= nwrite;
	}
	return 0;
}

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
"@(#) Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static const char sccsid[] = "from: @(#)wall.c	5.14 (Berkeley) 3/2/91";
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * This program is not related to David Wall, whose Stanford Ph.D. thesis
 * is entitled "Mechanisms for Broadcast and Selective Broadcast".
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <rpc/rpc.h>
#include <rpcsvc/rwall.h>
#include <err.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static char *mbuf;

static char notty[] = "no tty";

static void	makemsg(const char *);
static void usage(void);

/* ARGSUSED */
int
main(int argc, char *argv[])
{
	char *wallhost, res;
	CLIENT *cl;
	struct timeval tv;

	if ((argc < 2) || (argc > 3))
		usage();

	wallhost = argv[1];

	makemsg(argv[2]);

	/*
	 * Create client "handle" used for calling MESSAGEPROG on the
	 * server designated on the command line. We tell the rpc package
	 * to use the "tcp" protocol when contacting the server.
	*/
	cl = clnt_create(wallhost, WALLPROG, WALLVERS, "udp");
	if (cl == NULL) {
		/*
		 * Couldn't establish connection with server.
		 * Print error message and die.
		 */
		errx(1, "%s", clnt_spcreateerror(wallhost));
	}

	tv.tv_sec = 15;		/* XXX ?? */
	tv.tv_usec = 0;
	if (clnt_call(cl, WALLPROC_WALL, (xdrproc_t)xdr_wrapstring, &mbuf,
	    (xdrproc_t)xdr_void, &res, tv) != RPC_SUCCESS) {
		/*
		 * An error occurred while calling the server.
		 * Print error message and die.
		 */
		errx(1, "%s", clnt_sperror(cl, wallhost));
	}

	return (0);
}

static void
usage(void)
{
	fprintf(stderr, "usage: rwall host [file]\n");
	exit(1);
}

static void
makemsg(const char *fname)
{
	struct tm *lt;
	struct passwd *pw;
	struct stat sbuf;
	time_t now;
	FILE *fp;
	int fd;
	size_t mbufsize;
	char *tty, hostname[MAXHOSTNAMELEN], lbuf[256], tmpname[64];
	const char *whom;

	snprintf(tmpname, sizeof(tmpname), "%s/wall.XXXXXX", _PATH_TMP);
	if ((fd = mkstemp(tmpname)) == -1 || (fp = fdopen(fd, "r+")) == NULL)
		err(1, "can't open temporary file");
	unlink(tmpname);

	whom = getlogin();
	if (!whom) {
		pw = getpwuid(getuid());
		whom = pw ? pw->pw_name : "???";
	}
	gethostname(hostname, sizeof(hostname));
	time(&now);
	lt = localtime(&now);

	/*
	 * all this stuff is to blank out a square for the message;
	 * we wrap message lines at column 79, not 80, because some
	 * terminals wrap after 79, some do not, and we can't tell.
	 * Which means that we may leave a non-blank character
	 * in column 80, but that can't be helped.
	 */
	fprintf(fp, "Remote Broadcast Message from %s@%s\n",
	    whom, hostname);
	tty = ttyname(STDERR_FILENO);
	if (tty == NULL)
		tty = notty;
	fprintf(fp, "        (%s) at %d:%02d ...\n", tty,
	    lt->tm_hour, lt->tm_min);

	putc('\n', fp);

	if (fname && !(freopen(fname, "r", stdin)))
		err(1, "can't read %s", fname);
	while (fgets(lbuf, sizeof(lbuf), stdin))
		fputs(lbuf, fp);
	rewind(fp);

	if (fstat(fd, &sbuf))
		err(1, "can't stat temporary file");
	mbufsize = (size_t)sbuf.st_size;
	mbuf = malloc(mbufsize);
	if (mbuf == NULL)
		err(1, "out of memory");
	if (fread(mbuf, sizeof(*mbuf), mbufsize, fp) != (u_int)mbufsize)
		err(1, "can't read temporary file");
	close(fd);
}
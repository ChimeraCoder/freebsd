
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
#if 0
static const char sccsid[] = "@(#)util.c	8.2 (Berkeley) 4/2/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

char *
colon(char *cp)
{
	if (*cp == ':')		/* Leading colon is part of file name. */
		return (0);

	for (; *cp; ++cp) {
		if (*cp == ':')
			return (cp);
		if (*cp == '/')
			return (0);
	}
	return (0);
}

void
verifydir(char *cp)
{
	struct stat stb;

	if (!stat(cp, &stb)) {
		if (S_ISDIR(stb.st_mode))
			return;
		errno = ENOTDIR;
	}
	run_err("%s: %s", cp, strerror(errno));
	exit(1);
}

int
okname(char *cp0)
{
	int c;
	char *cp;

	cp = cp0;
	do {
		c = *cp;
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-' && c != '.')
			goto bad;
	} while (*++cp);
	return (1);

bad:	warnx("%s: invalid user name", cp0);
	return (0);
}

int
susystem(char *s, int userid)
{
	sig_t istat, qstat;
	int status;
	pid_t pid;

	pid = vfork();
	switch (pid) {
	case -1:
		return (127);

	case 0:
		(void)setuid(userid);
		execl(_PATH_BSHELL, "sh", "-c", s, (char *)NULL);
		_exit(127);
	}
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	if (waitpid(pid, &status, 0) < 0)
		status = -1;
	(void)signal(SIGINT, istat);
	(void)signal(SIGQUIT, qstat);
	return (status);
}

BUF *
allocbuf(BUF *bp, int fd, int blksize)
{
	struct stat stb;
	size_t size;

	if (fstat(fd, &stb) < 0) {
		run_err("fstat: %s", strerror(errno));
		return (0);
	}
	size = roundup(stb.st_blksize, blksize);
	if (size == 0)
		size = blksize;
	if (bp->cnt >= size)
		return (bp);
	if ((bp->buf = realloc(bp->buf, size)) == NULL) {
		bp->cnt = 0;
		run_err("%s", strerror(errno));
		return (0);
	}
	bp->cnt = size;
	return (bp);
}

void
lostconn(int signo __unused)
{
	if (!iamremote)
		warnx("lost connection");
	exit(1);
}
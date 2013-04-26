
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
#if 0
static char sccsid[] = "@(#)util.c	8.2 (Berkeley) 4/2/94";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */
#endif

#include "rcp_locl.h"

RCSID("$Id$");

char *
colon(cp)
	char *cp;
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

char *
unbracket(char *cp)
{
	char *ep;

	if (*cp == '[') {
		ep = cp + (strlen(cp) - 1);
		if (*ep == ']') {
			*ep = '\0';
			++cp;
		}
	}
	return (cp);
}

void
verifydir(cp)
	char *cp;
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
okname(cp0)
	char *cp0;
{
	int c;
	unsigned char *cp;

	cp = (unsigned char *)cp0;
	do {
		c = *cp;
		if (c & 0200)
			goto bad;
		if (!isalpha(c) && !isdigit(c) && c != '_' && c != '-')
			goto bad;
	} while (*++cp);
	return (1);

bad:	warnx("%s: invalid user name", cp0);
	return (0);
}

int
susystem(s)
	char *s;
{
	void (*istat)(int), (*qstat)(int);
	int status;
	pid_t pid;

	pid = fork();
	switch (pid) {
	case -1:
		return (127);

	case 0:
		execl(_PATH_BSHELL, "sh", "-c", s, NULL);
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

#ifndef roundup
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))
#endif

BUF *
allocbuf(bp, fd, blksize)
	BUF *bp;
	int fd, blksize;
{
	struct stat stb;
	size_t size;
	char *p;

	if (fstat(fd, &stb) < 0) {
		run_err("fstat: %s", strerror(errno));
		return (0);
	}
	size = roundup(stb.st_blksize, blksize);
	if (size == 0)
		size = blksize;
	if (bp->cnt >= size)
		return (bp);
	if ((p = realloc(bp->buf, size)) == NULL) {
		if (bp->buf)
			free(bp->buf);
		bp->buf = NULL;
		bp->cnt = 0;
		run_err("%s", strerror(errno));
		return (0);
	}
	memset(p, 0, size);
	bp->buf = p;
	bp->cnt = size;
	return (bp);
}

void
lostconn(signo)
	int signo;
{
	if (!iamremote)
		warnx("lost connection");
	exit(1);
}
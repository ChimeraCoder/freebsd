
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)fingerd.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>
#include <syslog.h>
#include <libutil.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pathnames.h"

void logerr(const char *, ...) __printflike(1, 2) __dead2;

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ch;
	char *lp;
	struct sockaddr_storage ss;
	socklen_t sval;
	int p[2], debug, kflag, logging, pflag, secure;
#define	ENTRIES	50
	char **ap, *av[ENTRIES + 1], **comp, line[1024], *prog;
	char rhost[MAXHOSTNAMELEN];

	prog = _PATH_FINGER;
	debug = logging = kflag = pflag = secure = 0;
	openlog("fingerd", LOG_PID | LOG_CONS, LOG_DAEMON);
	opterr = 0;
	while ((ch = getopt(argc, argv, "dklp:s")) != -1)
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 'k':
			kflag = 1;
			break;
		case 'l':
			logging = 1;
			break;
		case 'p':
			prog = optarg;
			pflag = 1;
			break;
		case 's':
			secure = 1;
			break;
		case '?':
		default:
			logerr("illegal option -- %c", optopt);
		}

	/*
	 * Enable server-side Transaction TCP.
	 */
	if (!debug) {
		int one = 1;
		if (setsockopt(STDOUT_FILENO, IPPROTO_TCP, TCP_NOPUSH, &one, 
			       sizeof one) < 0) {
			logerr("setsockopt(TCP_NOPUSH) failed: %m");
		}
	}

	if (!fgets(line, sizeof(line), stdin))
		exit(1);

	if (!debug && (logging || pflag)) {
		sval = sizeof(ss);
		if (getpeername(0, (struct sockaddr *)&ss, &sval) < 0)
			logerr("getpeername: %s", strerror(errno));
		realhostname_sa(rhost, sizeof rhost - 1,
				(struct sockaddr *)&ss, sval);
		rhost[sizeof(rhost) - 1] = '\0';
		if (pflag)
			setenv("FINGERD_REMOTE_HOST", rhost, 1);
	}

	if (logging) {
		char *t;
		char *end;

		end = memchr(line, 0, sizeof(line));
		if (end == NULL) {
			if ((t = malloc(sizeof(line) + 1)) == NULL)
				logerr("malloc: %s", strerror(errno));
			memcpy(t, line, sizeof(line));
			t[sizeof(line)] = 0;
		} else {
			if ((t = strdup(line)) == NULL)
				logerr("strdup: %s", strerror(errno));
		}
		for (end = t; *end; end++)
			if (*end == '\n' || *end == '\r')
				*end = ' ';
		syslog(LOG_NOTICE, "query from %s: `%s'", rhost, t);
	}

	comp = &av[2];
	av[3] = "--";
	if (kflag)
		*comp-- = "-k";
	for (lp = line, ap = &av[4];;) {
		*ap = strtok(lp, " \t\r\n");
		if (!*ap) {
			if (secure && ap == &av[4]) {
				puts("must provide username\r\n");
				exit(1);
			}
			break;
		}
		if (secure && strchr(*ap, '@')) {
			puts("forwarding service denied\r\n");
			exit(1);
		}

		/* RFC742: "/[Ww]" == "-l" */
		if ((*ap)[0] == '/' && ((*ap)[1] == 'W' || (*ap)[1] == 'w')) {
			*comp-- = "-l";
		}
		else if (++ap == av + ENTRIES) {
			*ap = NULL;
			break;
		}
		lp = NULL;
	}

	if ((lp = strrchr(prog, '/')) != NULL)
		*comp = ++lp;
	else
		*comp = prog;
	if (pipe(p) < 0)
		logerr("pipe: %s", strerror(errno));

	if (debug) {
		fprintf(stderr, "%s", prog);
		for (ap = comp; *ap != NULL; ++ap)
			fprintf(stderr, " %s", *ap);
		fprintf(stderr, "\n");
	}

	switch(vfork()) {
	case 0:
		(void)close(p[0]);
		if (p[1] != STDOUT_FILENO) {
			(void)dup2(p[1], STDOUT_FILENO);
			(void)close(p[1]);
		}
		dup2(STDOUT_FILENO, STDERR_FILENO);

		execv(prog, comp);
		write(STDERR_FILENO, prog, strlen(prog));
#define MSG ": cannot execute\n"
		write(STDERR_FILENO, MSG, strlen(MSG));
#undef MSG
		_exit(1);
	case -1:
		logerr("fork: %s", strerror(errno));
	}
	(void)close(p[1]);
	if (!(fp = fdopen(p[0], "r")))
		logerr("fdopen: %s", strerror(errno));
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')
			putchar('\r');
		putchar(ch);
	}
	exit(0);
}

#include <stdarg.h>

void
logerr(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	(void)vsyslog(LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
	/* NOTREACHED */
}
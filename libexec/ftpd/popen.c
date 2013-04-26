
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
static char sccsid[] = "@(#)popen.c	8.3 (Berkeley) 4/6/94";
#endif
#endif /* not lint */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <errno.h>
#include <glob.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"
#include "pathnames.h"
#include <syslog.h>
#include <time.h>

#define	MAXUSRARGS	100
#define	MAXGLOBARGS	1000

/*
 * Special version of popen which avoids call to shell.  This ensures noone
 * may create a pipe to a hidden program as a side effect of a list or dir
 * command.
 */
static int *pids;
static int fds;

FILE *
ftpd_popen(char *program, char *type)
{
	char *cp;
	FILE *iop;
	int argc, gargc, pdes[2], pid;
	char **pop, *argv[MAXUSRARGS], *gargv[MAXGLOBARGS];

	if (((*type != 'r') && (*type != 'w')) || type[1])
		return (NULL);

	if (!pids) {
		if ((fds = getdtablesize()) <= 0)
			return (NULL);
		if ((pids = malloc(fds * sizeof(int))) == NULL)
			return (NULL);
		memset(pids, 0, fds * sizeof(int));
	}
	if (pipe(pdes) < 0)
		return (NULL);

	/* break up string into pieces */
	for (argc = 0, cp = program; argc < MAXUSRARGS; cp = NULL) {
		if (!(argv[argc++] = strtok(cp, " \t\n")))
			break;
	}
	argv[argc - 1] = NULL;

	/* glob each piece */
	gargv[0] = argv[0];
	for (gargc = argc = 1; argv[argc] && gargc < (MAXGLOBARGS-1); argc++) {
		glob_t gl;
		int flags = GLOB_BRACE|GLOB_NOCHECK|GLOB_TILDE;

		memset(&gl, 0, sizeof(gl));
		gl.gl_matchc = MAXGLOBARGS;
		flags |= GLOB_LIMIT;
		if (glob(argv[argc], flags, NULL, &gl))
			gargv[gargc++] = strdup(argv[argc]);
		else if (gl.gl_pathc > 0) {
			for (pop = gl.gl_pathv; *pop && gargc < (MAXGLOBARGS-1);
			     pop++)
				gargv[gargc++] = strdup(*pop);
		}
		globfree(&gl);
	}
	gargv[gargc] = NULL;

	iop = NULL;
	fflush(NULL);
	pid = (strcmp(gargv[0], _PATH_LS) == 0) ? fork() : vfork();
	switch(pid) {
	case -1:			/* error */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		goto pfree;
		/* NOTREACHED */
	case 0:				/* child */
		if (*type == 'r') {
			if (pdes[1] != STDOUT_FILENO) {
				dup2(pdes[1], STDOUT_FILENO);
				(void)close(pdes[1]);
			}
			dup2(STDOUT_FILENO, STDERR_FILENO); /* stderr too! */
			(void)close(pdes[0]);
		} else {
			if (pdes[0] != STDIN_FILENO) {
				dup2(pdes[0], STDIN_FILENO);
				(void)close(pdes[0]);
			}
			(void)close(pdes[1]);
		}
		/* Drop privileges before proceeding */
		if (getuid() != geteuid() && setuid(geteuid()) < 0)
			_exit(1);
		if (strcmp(gargv[0], _PATH_LS) == 0) {
			/* Reset getopt for ls_main() */
			optreset = optind = optopt = 1;
			/* Close syslogging to remove pwd.db missing msgs */
			closelog();
			/* Trigger to sense new /etc/localtime after chroot */
			if (getenv("TZ") == NULL) {
				setenv("TZ", "", 0);
				tzset();
				unsetenv("TZ");
				tzset();
			}
			exit(ls_main(gargc, gargv));
		}
		execv(gargv[0], gargv);
		_exit(1);
	}
	/* parent; assume fdopen can't fail...  */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}
	pids[fileno(iop)] = pid;

pfree:	for (argc = 1; gargv[argc] != NULL; argc++)
		free(gargv[argc]);

	return (iop);
}

int
ftpd_pclose(FILE *iop)
{
	int fdes, omask, status;
	pid_t pid;

	/*
	 * pclose returns -1 if stream is not associated with a
	 * `popened' command, or, if already `pclosed'.
	 */
	if (pids == 0 || pids[fdes = fileno(iop)] == 0)
		return (-1);
	(void)fclose(iop);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = waitpid(pids[fdes], &status, 0)) < 0 && errno == EINTR)
		continue;
	(void)sigsetmask(omask);
	pids[fdes] = 0;
	if (pid < 0)
		return (pid);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	return (1);
}
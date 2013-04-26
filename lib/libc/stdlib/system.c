
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)system.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <paths.h>
#include <errno.h>
#include "un-namespace.h"
#include "libc_private.h"

int
__system(const char *command)
{
	pid_t pid, savedpid;
	int pstat;
	struct sigaction ign, intact, quitact;
	sigset_t newsigblock, oldsigblock;

	if (!command)		/* just checking... */
		return(1);

	/*
	 * Ignore SIGINT and SIGQUIT, block SIGCHLD. Remember to save
	 * existing signal dispositions.
	 */
	ign.sa_handler = SIG_IGN;
	(void)sigemptyset(&ign.sa_mask);
	ign.sa_flags = 0;
	(void)_sigaction(SIGINT, &ign, &intact);
	(void)_sigaction(SIGQUIT, &ign, &quitact);
	(void)sigemptyset(&newsigblock);
	(void)sigaddset(&newsigblock, SIGCHLD);
	(void)_sigprocmask(SIG_BLOCK, &newsigblock, &oldsigblock);
	switch(pid = fork()) {
	case -1:			/* error */
		break;
	case 0:				/* child */
		/*
		 * Restore original signal dispositions and exec the command.
		 */
		(void)_sigaction(SIGINT, &intact, NULL);
		(void)_sigaction(SIGQUIT,  &quitact, NULL);
		(void)_sigprocmask(SIG_SETMASK, &oldsigblock, NULL);
		execl(_PATH_BSHELL, "sh", "-c", command, (char *)NULL);
		_exit(127);
	default:			/* parent */
		savedpid = pid;
		do {
			pid = _wait4(savedpid, &pstat, 0, (struct rusage *)0);
		} while (pid == -1 && errno == EINTR);
		break;
	}
	(void)_sigaction(SIGINT, &intact, NULL);
	(void)_sigaction(SIGQUIT,  &quitact, NULL);
	(void)_sigprocmask(SIG_SETMASK, &oldsigblock, NULL);
	return(pid == -1 ? -1 : pstat);
}

__weak_reference(__system, system);
__weak_reference(__system, _system);
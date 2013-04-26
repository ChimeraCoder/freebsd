
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/wait.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>
#include "ulog.h"

#define	_PATH_ULOG_HELPER	"/usr/libexec/ulog-helper"

/*
 * Registering login sessions.
 */

static void
ulog_exec_helper(int fd, char const * const argv[])
{
	sigset_t oblock, nblock;
	pid_t pid, wpid;
	int status;

	/* Block SIGCHLD. */
	sigemptyset(&nblock);
	sigaddset(&nblock, SIGCHLD);
	sigprocmask(SIG_BLOCK, &nblock, &oblock);

	switch (pid = fork()) {
	case -1:
		break;
	case 0:
		/* Execute helper program. */
		if (dup2(fd, STDIN_FILENO) == -1)
			exit(EX_UNAVAILABLE);
		sigprocmask(SIG_SETMASK, &oblock, NULL);
		execv(_PATH_ULOG_HELPER, __DECONST(char * const *, argv));
		exit(EX_UNAVAILABLE);
	default:
		/* Wait for helper to finish. */
		do {
			wpid = waitpid(pid, &status, 0);
		} while (wpid == -1 && errno == EINTR);
		break;
	}

	sigprocmask(SIG_SETMASK, &oblock, NULL);
}

void
ulog_login_pseudo(int fd, const char *host)
{
	char const * const args[4] = { "ulog-helper", "login", host, NULL };

	ulog_exec_helper(fd, args);
}

void
ulog_logout_pseudo(int fd)
{
	char const * const args[3] = { "ulog-helper", "logout", NULL };

	ulog_exec_helper(fd, args);
}
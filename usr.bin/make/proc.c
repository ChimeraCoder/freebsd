
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

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "proc.h"
#include "shell.h"
#include "util.h"

/**
 * Replace the current process.
 */
void
Proc_Exec(const ProcStuff *ps)
{

	if (ps->in != STDIN_FILENO) {
		/*
		 * Redirect the child's stdin to the input fd
		 * and reset it to the beginning (again).
		 */
		if (dup2(ps->in, STDIN_FILENO) == -1)
			Punt("Cannot dup2: %s", strerror(errno));
		lseek(STDIN_FILENO, (off_t)0, SEEK_SET);
	}

	if (ps->out != STDOUT_FILENO) {
		/*
		 * Redirect the child's stdout to the output fd.
		 */
		if (dup2(ps->out, STDOUT_FILENO) == -1)
			Punt("Cannot dup2: %s", strerror(errno));
		close(ps->out);
	}

	if (ps->err != STDERR_FILENO) {
		/*
		 * Redirect the child's stderr to the err fd.
		 */
		if (dup2(ps->err, STDERR_FILENO) == -1)
			Punt("Cannot dup2: %s", strerror(errno));
		close(ps->err);
	}

	if (ps->merge_errors) {
		/*
		 * Send stderr to parent process too.
		 */
		if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1)
			Punt("Cannot dup2: %s", strerror(errno));
	}

	if (commandShell->unsetenv) {
		/* for the benfit of ksh */
		unsetenv("ENV");
	}

	/*
	 * The file descriptors for stdin, stdout, or stderr might
	 * have been marked close-on-exec.  Clear the flag on all
	 * of them.
	 */
	fcntl(STDIN_FILENO, F_SETFD,
	    fcntl(STDIN_FILENO, F_GETFD) & (~FD_CLOEXEC));
	fcntl(STDOUT_FILENO, F_SETFD,
	    fcntl(STDOUT_FILENO, F_GETFD) & (~FD_CLOEXEC));
	fcntl(STDERR_FILENO, F_SETFD,
	    fcntl(STDERR_FILENO, F_GETFD) & (~FD_CLOEXEC));

	if (ps->pgroup) {
#ifdef USE_PGRP
		/*
		 * Become a process group leader, so we can kill it and all
		 * its descendants in one fell swoop, by killing its process
		 * family, but not commit suicide.
		 */
#if defined(SYSV)
		setsid();
#else
		setpgid(0, getpid());
#endif
#endif /* USE_PGRP */
	}

	if (ps->searchpath) {
		execvp(ps->argv[0], ps->argv);

		write(STDERR_FILENO, ps->argv[0], strlen(ps->argv[0]));
		write(STDERR_FILENO, ": ", 2);
		write(STDERR_FILENO, strerror(errno), strlen(strerror(errno)));
		write(STDERR_FILENO, "\n", 1);
	} else {
		execv(commandShell->path, ps->argv);

		write(STDERR_FILENO,
		    "Could not execute shell\n",
		    sizeof("Could not execute shell"));
	}

	/*
	 * Since we are the child process, exit without flushing buffers.
	 */
	_exit(1);
}

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

#include "_libproc.h"
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int
proc_attach(pid_t pid, int flags, struct proc_handle **pphdl)
{
	struct proc_handle *phdl;
	int error = 0;
	int status;

	if (pid == 0 || pid == getpid())
		return (EINVAL);

	/*
	 * Allocate memory for the process handle, a structure containing
	 * all things related to the process.
	 */
	if ((phdl = malloc(sizeof(struct proc_handle))) == NULL)
		return (ENOMEM);

	memset(phdl, 0, sizeof(struct proc_handle));
	phdl->pid = pid;
	phdl->flags = flags;
	phdl->status = PS_RUN;
	elf_version(EV_CURRENT);

	if (ptrace(PT_ATTACH, phdl->pid, 0, 0) != 0) {
		error = errno;
		DPRINTF("ERROR: cannot ptrace child process %d", pid);
		goto out;
	}

	/* Wait for the child process to stop. */
	if (waitpid(pid, &status, WUNTRACED) == -1) {
		error = errno;
		DPRINTF("ERROR: child process %d didn't stop as expected", pid);
		goto out;
	}

	/* Check for an unexpected status. */
	if (WIFSTOPPED(status) == 0)
		DPRINTF("ERROR: child process %d status 0x%x", pid, status);
	else
		phdl->status = PS_STOP;

out:
	if (error)
		proc_free(phdl);
	else
		*pphdl = phdl;
	return (error);
}

int
proc_create(const char *file, char * const *argv, proc_child_func *pcf,
    void *child_arg, struct proc_handle **pphdl)
{
	struct proc_handle *phdl;
	int error = 0;
	int status;
	pid_t pid;

	/*
	 * Allocate memory for the process handle, a structure containing
	 * all things related to the process.
	 */
	if ((phdl = malloc(sizeof(struct proc_handle))) == NULL)
		return (ENOMEM);

	elf_version(EV_CURRENT);

	/* Fork a new process. */
	if ((pid = vfork()) == -1)
		error = errno;
	else if (pid == 0) {
		/* The child expects to be traced. */
		if (ptrace(PT_TRACE_ME, 0, 0, 0) != 0)
			_exit(1);

		if (pcf != NULL)
			(*pcf)(child_arg);

		/* Execute the specified file: */
		execvp(file, argv);

		/* Couldn't execute the file. */
		_exit(2);
	} else {
		/* The parent owns the process handle. */
		memset(phdl, 0, sizeof(struct proc_handle));
		phdl->pid = pid;
		phdl->status = PS_IDLE;

		/* Wait for the child process to stop. */
		if (waitpid(pid, &status, WUNTRACED) == -1) {
			error = errno;
                	DPRINTF("ERROR: child process %d didn't stop as expected", pid);
			goto bad;
		}

		/* Check for an unexpected status. */
		if (WIFSTOPPED(status) == 0) {
			error = errno;
                	DPRINTF("ERROR: child process %d status 0x%x", pid, status);
			goto bad;
		} else
			phdl->status = PS_STOP;
	}
bad:
	if (error)
		proc_free(phdl);
	else
		*pphdl = phdl;
	return (error);
}

void
proc_free(struct proc_handle *phdl)
{
	free(phdl);
}
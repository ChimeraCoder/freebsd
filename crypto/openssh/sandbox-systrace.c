
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

#include "includes.h"

#ifdef SANDBOX_SYSTRACE

#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <dev/systrace.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "atomicio.h"
#include "log.h"
#include "ssh-sandbox.h"
#include "xmalloc.h"

struct sandbox_policy {
	int syscall;
	int action;
};

/* Permitted syscalls in preauth. Unlisted syscalls get SYSTR_POLICY_KILL */
static const struct sandbox_policy preauth_policy[] = {
	{ SYS_open, SYSTR_POLICY_NEVER },

	{ SYS___sysctl, SYSTR_POLICY_PERMIT },
	{ SYS_close, SYSTR_POLICY_PERMIT },
	{ SYS_exit, SYSTR_POLICY_PERMIT },
	{ SYS_getpid, SYSTR_POLICY_PERMIT },
	{ SYS_gettimeofday, SYSTR_POLICY_PERMIT },
	{ SYS_madvise, SYSTR_POLICY_PERMIT },
	{ SYS_mmap, SYSTR_POLICY_PERMIT },
	{ SYS_mprotect, SYSTR_POLICY_PERMIT },
	{ SYS_mquery, SYSTR_POLICY_PERMIT },
	{ SYS_poll, SYSTR_POLICY_PERMIT },
	{ SYS_munmap, SYSTR_POLICY_PERMIT },
	{ SYS_read, SYSTR_POLICY_PERMIT },
	{ SYS_select, SYSTR_POLICY_PERMIT },
	{ SYS_sigprocmask, SYSTR_POLICY_PERMIT },
	{ SYS_write, SYSTR_POLICY_PERMIT },
	{ -1, -1 }
};

struct ssh_sandbox {
	int systrace_fd;
	pid_t child_pid;
	void (*osigchld)(int);
};

struct ssh_sandbox *
ssh_sandbox_init(void)
{
	struct ssh_sandbox *box;

	debug3("%s: preparing systrace sandbox", __func__);
	box = xcalloc(1, sizeof(*box));
	box->systrace_fd = -1;
	box->child_pid = 0;
	box->osigchld = signal(SIGCHLD, SIG_IGN);

	return box;
}

void
ssh_sandbox_child(struct ssh_sandbox *box)
{
	debug3("%s: ready", __func__);
	signal(SIGCHLD, box->osigchld);
	if (kill(getpid(), SIGSTOP) != 0)
		fatal("%s: kill(%d, SIGSTOP)", __func__, getpid());
	debug3("%s: started", __func__);
}

static void
ssh_sandbox_parent(struct ssh_sandbox *box, pid_t child_pid,
    const struct sandbox_policy *allowed_syscalls)
{
	int dev_systrace, i, j, found, status;
	pid_t pid;
	struct systrace_policy policy;

	/* Wait for the child to send itself a SIGSTOP */
	debug3("%s: wait for child %ld", __func__, (long)child_pid);
	do {
		pid = waitpid(child_pid, &status, WUNTRACED);
	} while (pid == -1 && errno == EINTR);
	signal(SIGCHLD, box->osigchld);
	if (!WIFSTOPPED(status)) {
		if (WIFSIGNALED(status))
			fatal("%s: child terminated with signal %d",
			    __func__, WTERMSIG(status));
		if (WIFEXITED(status))
			fatal("%s: child exited with status %d",
			    __func__, WEXITSTATUS(status));
		fatal("%s: child not stopped", __func__);
	}
	debug3("%s: child %ld stopped", __func__, (long)child_pid);
	box->child_pid = child_pid;

	/* Set up systracing of child */
	if ((dev_systrace = open("/dev/systrace", O_RDONLY)) == -1)
		fatal("%s: open(\"/dev/systrace\"): %s", __func__,
		    strerror(errno));
	if (ioctl(dev_systrace, STRIOCCLONE, &box->systrace_fd) == -1)
		fatal("%s: ioctl(STRIOCCLONE, %d): %s", __func__,
		    dev_systrace, strerror(errno));
	close(dev_systrace);
	debug3("%s: systrace attach, fd=%d", __func__, box->systrace_fd);
	if (ioctl(box->systrace_fd, STRIOCATTACH, &child_pid) == -1)
		fatal("%s: ioctl(%d, STRIOCATTACH, %d): %s", __func__,
		    box->systrace_fd, child_pid, strerror(errno));

	/* Allocate and assign policy */
	bzero(&policy, sizeof(policy));
	policy.strp_op = SYSTR_POLICY_NEW;
	policy.strp_maxents = SYS_MAXSYSCALL;
	if (ioctl(box->systrace_fd, STRIOCPOLICY, &policy) == -1)
		fatal("%s: ioctl(%d, STRIOCPOLICY (new)): %s", __func__,
		    box->systrace_fd, strerror(errno));

	policy.strp_op = SYSTR_POLICY_ASSIGN;
	policy.strp_pid = box->child_pid;
	if (ioctl(box->systrace_fd, STRIOCPOLICY, &policy) == -1)
		fatal("%s: ioctl(%d, STRIOCPOLICY (assign)): %s",
		    __func__, box->systrace_fd, strerror(errno));

	/* Set per-syscall policy */
	for (i = 0; i < SYS_MAXSYSCALL; i++) {
		found = 0;
		for (j = 0; allowed_syscalls[j].syscall != -1; j++) {
			if (allowed_syscalls[j].syscall == i) {
				found = 1;
				break;
			}
		}
		policy.strp_op = SYSTR_POLICY_MODIFY;
		policy.strp_code = i;
		policy.strp_policy = found ?
		    allowed_syscalls[j].action : SYSTR_POLICY_KILL;
		if (found)
			debug3("%s: policy: enable syscall %d", __func__, i);
		if (ioctl(box->systrace_fd, STRIOCPOLICY, &policy) == -1)
			fatal("%s: ioctl(%d, STRIOCPOLICY (modify)): %s",
			    __func__, box->systrace_fd, strerror(errno));
	}

	/* Signal the child to start running */
	debug3("%s: start child %ld", __func__, (long)child_pid);
	if (kill(box->child_pid, SIGCONT) != 0)
		fatal("%s: kill(%d, SIGCONT)", __func__, box->child_pid);
}

void
ssh_sandbox_parent_finish(struct ssh_sandbox *box)
{
	/* Closing this before the child exits will terminate it */
	close(box->systrace_fd);

	free(box);
	debug3("%s: finished", __func__);
}

void
ssh_sandbox_parent_preauth(struct ssh_sandbox *box, pid_t child_pid)
{
	ssh_sandbox_parent(box, child_pid, preauth_policy);
}

#endif /* SANDBOX_SYSTRACE */

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

/*
 * Test that privilege is required to lower nice value; first test with, then
 * without.  There are two failure modes associated with privilege: the right
 * to renice a process with a different uid, and the right to renice to a
 * lower priority.  Because both the real and effective uid are part of the
 * permissions test, we have to create two children processes with different
 * uids.
 */

#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "main.h"

static int	childproc_running;
static pid_t	childproc;

int
priv_sched_setpriority_setup(int asroot, int injail, struct test *test)
{
	int another_uid, need_child;

	/*
	 * Some tests require a second process with specific credentials.
	 * Set that up here, and kill in cleanup.
	 */
	need_child = 0;
	if (test->t_test_func == priv_sched_setpriority_aproc) {
		need_child = 1;
		another_uid = 1;
	}
	if (test->t_test_func == priv_sched_setpriority_myproc)
		need_child = 1;

	if (need_child) {
		childproc = fork();
		if (childproc < 0) {
			warn("priv_sched_setup: fork");
			return (-1);
		}
		if (childproc == 0) {
			if (another_uid) {
				if (setresuid(UID_THIRD, UID_THIRD,
				    UID_THIRD) < 0)
				err(-1, "setresuid(%d)", UID_THIRD);
			}
			while (1)
				sleep(1);
		}
		childproc_running = 1;
		sleep(1);	/* Allow dummy thread to change uids. */
	}
	return (0);
}

void
priv_sched_setpriority_curproc(int asroot, int injail, struct test *test)
{
	int error;

	error = setpriority(PRIO_PROCESS, 0, -1);
	if (asroot && injail)
		expect("priv_sched_setpriority_curproc(asroot, injail)",
		    error, -1, EACCES);
	if (asroot && !injail)
		expect("priv_sched_setpriority_curproc(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_setpriority_curproc(!asroot, injail)",
		    error, -1, EACCES);
	if (!asroot && !injail)
		expect("priv_sched_setpriority_curproc(!asroot, !injail)",
		    error, -1, EACCES);
}

void
priv_sched_setpriority_myproc(int asroot, int injail, struct test *test)
{
	int error;

	error = setpriority(PRIO_PROCESS, 0, -1);
	if (asroot && injail)
		expect("priv_sched_setpriority_myproc(asroot, injail)",
		    error, -1, EACCES);
	if (asroot && !injail)
		expect("priv_sched_setpriority_myproc(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_setpriority_myproc(!asroot, injail)",
		    error, -1, EACCES);
	if (!asroot && !injail)
		expect("priv_sched_setpriority_myproc(!asroot, !injail)",
		    error, -1, EACCES);
}

void
priv_sched_setpriority_aproc(int asroot, int injail, struct test *test)
{
	int error;

	error = setpriority(PRIO_PROCESS, 0, -1);
	if (asroot && injail)
		expect("priv_sched_setpriority_aproc(asroot, injail)",
		    error, -1, EACCES);
	if (asroot && !injail)
		expect("priv_sched_setpriority_aproc(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_setpriority_aproc(!asroot, injail)",
		    error, -1, EACCES);
	if (!asroot && !injail)
		expect("priv_sched_setpriority_aproc(!asroot, !injail)",
		    error, -1, EACCES);
}

void
priv_sched_setpriority_cleanup(int asroot, int injail, struct test *test)
{
	pid_t pid;

	if (childproc_running) {
		(void)kill(childproc, SIGKILL);
		while (1) {
			pid = waitpid(childproc, NULL, 0);
			if (pid == -1)
				warn("waitpid(%d (test), NULL, 0)",
				    childproc);
			if (pid == childproc)
				break;
		}
		childproc_running = 0;
		childproc = -1;
	}
}
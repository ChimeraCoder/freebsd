
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
 * Test privilege associated with real time process settings.  There are
 * three relevant notions of privilege:
 *
 * - Privilege to set the real-time priority of the current process.
 * - Privilege to set the real-time priority of another process.
 * - Privilege to set the idle priority of another process.
 * - No privilege to set the idle priority of the current process.
 *
 * This requires a test process and a target (dummy) process running with
 * various uids.  This test is based on the code in the setpriority() test.
 */

#include <sys/types.h>
#include <sys/rtprio.h>
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
priv_sched_rtprio_setup(int asroot, int injail, struct test *test)
{
	int another_uid, need_child;

	/*
	 * Some tests require a second process with specific credentials.
	 * Set that up here, and kill in cleanup.
	 */
	need_child = 0;
	if (test->t_test_func == priv_sched_rtprio_aproc_normal ||
	    test->t_test_func == priv_sched_rtprio_aproc_idle ||
	    test->t_test_func == priv_sched_rtprio_aproc_realtime) {
		need_child = 1;
		another_uid = 1;
	}
	if (test->t_test_func == priv_sched_rtprio_myproc_normal ||
	    test->t_test_func == priv_sched_rtprio_myproc_idle ||
	    test->t_test_func == priv_sched_rtprio_myproc_realtime) {
		need_child = 1;
	}

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
priv_sched_rtprio_curproc_normal(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_NORMAL;
	rtp.prio = 0;
	error = rtprio(RTP_SET, 0, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_curproc_normal(asroot, injail)",
		    error, 0, 0);
	if (asroot && !injail)
		expect("priv_sched_rtprio_curproc_normal(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_curproc_normal(!asroot, injail)",
		    error, 0, 0);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_curproc_normal(!asroot, !injail)",
		    error, 0, 0);
}

void
priv_sched_rtprio_curproc_idle(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_IDLE;
	rtp.prio = 0;
	error = rtprio(RTP_SET, 0, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_curproc_idle(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_sched_rtprio_curproc_idle(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_curproc_idle(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_curproc_idle(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_curproc_realtime(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_REALTIME;
	rtp.prio = 0;
	error = rtprio(RTP_SET, 0, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_curproc_realtime(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_sched_rtprio_curproc_realtime(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_curproc_realtime(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_curproc_realtime(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_myproc_normal(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_NORMAL;
	rtp.prio = 0;
	error = rtprio(RTP_SET, 0, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_myproc_normal(asroot, injail)",
		    error, 0, 0);
	if (asroot && !injail)
		expect("priv_sched_rtprio_myproc_normal(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_myproc_normal(!asroot, injail)",
		    error, 0, 0);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_myproc_normal(!asroot, !injail)",
		    error, 0, 0);
}

void
priv_sched_rtprio_myproc_idle(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_IDLE;
	rtp.prio = 0;
	error = rtprio(RTP_SET, 0, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_myproc_idle(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_sched_rtprio_myproc_idle(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_myproc_idle(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_myproc_idle(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_myproc_realtime(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_REALTIME;
	rtp.prio = 0;
	error = rtprio(RTP_SET, 0, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_myproc_realtime(asroot, injail)",
		    error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_sched_rtprio_myproc_realtime(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_myproc_realtime(!asroot, injail)",
		    error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_myproc_realtime(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_aproc_normal(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_NORMAL;
	rtp.prio = 0;
	error = rtprio(RTP_SET, childproc, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_aproc_normal(asroot, injail)",
		    error, -1, ESRCH);
	if (asroot && !injail)
		expect("priv_sched_rtprio_aproc_normal(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_aproc_normal(!asroot, injail)",
		    error, -1, ESRCH);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_aproc_normal(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_aproc_idle(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_IDLE;
	rtp.prio = 0;
	error = rtprio(RTP_SET, childproc, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_aproc_idle(asroot, injail)",
		    error, -1, ESRCH);
	if (asroot && !injail)
		expect("priv_sched_rtprio_aproc_idle(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_aproc_idle(!asroot, injail)",
		    error, -1, ESRCH);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_aroc_idle(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_aproc_realtime(int asroot, int injail, struct test *test)
{
	struct rtprio rtp;
	int error;

	rtp.type = RTP_PRIO_REALTIME;
	rtp.prio = 0;
	error = rtprio(RTP_SET, childproc, &rtp);
	if (asroot && injail)
		expect("priv_sched_rtprio_aproc_realtime(asroot, injail)",
		    error, -1, ESRCH);
	if (asroot && !injail)
		expect("priv_sched_rtprio_aproc_realtime(asroot, !injail)",
		    error, 0, 0);
	if (!asroot && injail)
		expect("priv_sched_rtprio_aproc_realtime(!asroot, injail)",
		    error, -1, ESRCH);
	if (!asroot && !injail)
		expect("priv_sched_rtprio_aproc_realtime(!asroot, !injail)",
		    error, -1, EPERM);
}

void
priv_sched_rtprio_cleanup(int asroot, int injail, struct test *test)
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
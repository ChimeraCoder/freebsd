
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

#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUM_THREADS 100

static void *
vfork_test(void *threadid __unused)
{
	pid_t pid, wpid;
	int status;

	for (;;) {
		pid = vfork();
		if (pid == 0)
			_exit(0);
		else if (pid == -1)
			err(1, "Failed to vfork");
		else {
			wpid = waitpid(pid, &status, 0);
			if (wpid == -1)
				err(1, "waitpid");
		}
	}
	return (NULL);
}

static void
sighandler(int signo __unused)
{
}

/*
 * This program invokes multiple threads and each thread calls
 * vfork() system call.
 */
int
main(void)
{
	pthread_t threads[NUM_THREADS];
	struct sigaction reapchildren;
	sigset_t sigchld_mask;
	int rc, t;

	memset(&reapchildren, 0, sizeof(reapchildren));
	reapchildren.sa_handler = sighandler;
	if (sigaction(SIGCHLD, &reapchildren, NULL) == -1)
		err(1, "Could not sigaction(SIGCHLD)");

	sigemptyset(&sigchld_mask);
	sigaddset(&sigchld_mask, SIGCHLD);
	if (sigprocmask(SIG_BLOCK, &sigchld_mask, NULL) == -1)
		err(1, "sigprocmask");

	for (t = 0; t < NUM_THREADS; t++) {
		rc = pthread_create(&threads[t], NULL, vfork_test, &t);
		if (rc)
			errc(1, rc, "pthread_create");
	}
	pause();
	return (0);
}
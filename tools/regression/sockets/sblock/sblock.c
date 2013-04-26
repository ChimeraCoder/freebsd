
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
 * Sockets serialize I/O in each direction in order to avoid interlacing of
 * I/O by multiple processes or threcvs recving or sending the socket.  This
 * is done using some form of kernel lock (varies by kernel version), called
 * "sblock" in FreeBSD.  However, to avoid unkillable processes waiting on
 * I/O that may be entirely controlled by a remote network endpoint, that
 * lock acquisition must be interruptible.
 *
 * To test this, set up a local domain stream socket pair and a set of three
 * processes.  Two processes block in recv(), the first on sbwait (wait for
 * I/O), and the second on the sblock waiting for the first to finish.  A
 * third process is responsible for signalling the second process, then
 * writing to the socket.  Depending on the error returned in the second
 * process, we can tell whether the sblock wait was interrupted, or if
 * instead the process only woke up when the write was performed.
 */

#include <sys/socket.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int interrupted;
static void
signal_handler(int signum)
{

	interrupted++;
}

/*
 * Process that will perform a blocking recv on a UNIX domain socket.  This
 * should return one byte of data.
 */
static void
blocking_recver(int fd)
{
	ssize_t len;
	char ch;

	len = recv(fd, &ch, sizeof(ch), 0);
	if (len < 0)
		err(-1, "FAIL: blocking_recver: recv");
	if (len == 0)
		errx(-1, "FAIL: blocking_recver: recv: eof");
	if (len != 1)
		errx(-1, "FAIL: blocking_recver: recv: %zd bytes", len);
	if (interrupted)
		errx(-1, "FAIL: blocking_recver: interrupted wrong pid");
}

/*
 * Process that will perform a locking recv on a UNIX domain socket.
 *
 * This is where we figure out if the test worked or not.  If it has failed,
 * then recv() will return EOF, as the close() arrives before the signal,
 * meaning that the wait for the sblock was not interrupted; if it has
 * succeeded, we get EINTR as the signal interrupts the lock request.
 */
static void
locking_recver(int fd)
{
	ssize_t len;
	char ch;

	if (sleep(1) < 0)
		err(-1, "FAIL: locking_recver: sleep");
	len = recv(fd, &ch, sizeof(ch), 0);
	if (len < 0 && errno != EINTR)
		err(-1, "FAIL: locking_recver: recv");
	if (len < 0 && errno == EINTR) {
		fprintf(stderr, "PASS\n");
		exit(0);
	}
	if (len == 0)
		errx(-1, "FAIL: locking_recver: recv: eof");
	if (!interrupted)
		errx(-1, "FAIL: locking_recver: not interrupted");
}

static void
signaller(pid_t locking_recver_pid, int fd)
{
	ssize_t len;
	char ch;

	if (sleep(2) < 0) {
		warn("signaller sleep(2)");
		return;
	}
	if (kill(locking_recver_pid, SIGHUP) < 0) {
		warn("signaller kill(%d)", locking_recver_pid);
		return;
	}
	if (sleep(1) < 0) {
		warn("signaller sleep(1)");
		return;
	}
	len = send(fd, &ch, sizeof(ch), 0);
	if (len < 0) {
		warn("signaller send");
		return;
	}
	if (len != sizeof(ch)) {
		warnx("signaller send ret %zd", len);
		return;
	}
	if (close(fd) < 0) {
		warn("signaller close");
		return;
	}
	if (sleep(1) < 0) {
		warn("signaller sleep(1)");
		return;
	}
}

int
main(int argc, char *argv[])
{
	int error, fds[2], recver_fd, sender_fd;
	pid_t blocking_recver_pid;
	pid_t locking_recver_pid;
	struct sigaction sa;

	if (sigaction(SIGHUP, NULL, &sa) < 0)
		err(-1, "FAIL: sigaction(SIGHUP, NULL, &sa)");

	sa.sa_handler = signal_handler;
	if (sa.sa_flags & SA_RESTART)
		printf("SIGHUP restartable by default (cleared)\n");
	sa.sa_flags &= ~SA_RESTART;

	if (sigaction(SIGHUP, &sa, NULL) < 0)
		err(-1, "FAIL: sigaction(SIGHUP, &sa, NULL)");

#if 0
	if (signal(SIGHUP, signal_handler) == SIG_ERR)
		err(-1, "FAIL: signal(SIGHUP)");
#endif

	if (socketpair(PF_LOCAL, SOCK_STREAM, 0, fds) < 0)
		err(-1, "FAIL: socketpair(PF_LOCAL, SOGK_STREAM, 0)");

	sender_fd = fds[0];
	recver_fd = fds[1];

	blocking_recver_pid = fork();
	if (blocking_recver_pid < 0)
		err(-1, "FAIL: fork");
	if (blocking_recver_pid == 0) {
		close(sender_fd);
		blocking_recver(recver_fd);
		exit(0);
	}

	locking_recver_pid = fork();
	if (locking_recver_pid < 0) {
		error = errno;
		kill(blocking_recver_pid, SIGKILL);
		errno = error;
		err(-1, "FAIL: fork");
	}
	if (locking_recver_pid == 0) {
		close(sender_fd);
		locking_recver(recver_fd);
		exit(0);
	}

	signaller(locking_recver_pid, sender_fd);

	kill(blocking_recver_pid, SIGKILL);
	kill(locking_recver_pid, SIGKILL);
	exit(0);
}
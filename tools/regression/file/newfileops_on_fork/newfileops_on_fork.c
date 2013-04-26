
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
 * When a multi-threaded application calls fork(2) from one thread while
 * another thread is blocked in accept(2), we prefer that the file descriptor
 * to be returned by accept(2) not appear in the child process.  Test this by
 * creating a thread blocked in accept(2), then forking a child and seeing if
 * the fd it would have returned is defined in the child or not.
 */

#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	PORT	9000

static int listen_fd;

static void *
do_accept(__unused void *arg)
{
	int accept_fd;

	accept_fd = accept(listen_fd, NULL, NULL);
	if (accept_fd < 0)
		err(-1, "accept");

	return (NULL);
}

static void
do_fork(void)
{
	int pid;

	pid = fork();
	if (pid < 0)
		err(-1, "fork");
	if (pid > 0) {
		waitpid(pid, NULL, 0);
		exit(0);
	}

	/*
	 * We will call ftruncate(2) on the next available file descriptor,
	 * listen_fd+1, and get back EBADF if it's not a valid descriptor,
	 * and EINVAL if it is.  This (currently) works fine in practice.
	 */
	if (ftruncate(listen_fd + 1, 0 < 0)) {
		if (errno == EBADF)
			exit(0);
		else if (errno == EINVAL)
			errx(-1, "file descriptor still open in child");
		else
			err(-1, "unexpected error");
	} else
		errx(-1, "ftruncate succeeded");
}

int
main(__unused int argc, __unused char *argv[])
{
	struct sockaddr_in sin;
	pthread_t accept_thread;

	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err(-1, "socket");
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = htons(PORT);
	if (bind(listen_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		err(-1, "bind");
	if (listen(listen_fd, -1) <0)
		err(-1, "listen");
	if (pthread_create(&accept_thread, NULL, do_accept, NULL) != 0)
		err(-1, "pthread_create");
	sleep(1);	/* Easier than using a CV. */;
	do_fork();
	exit(0);
}
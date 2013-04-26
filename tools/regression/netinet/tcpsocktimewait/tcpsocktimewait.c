
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
 * TCP regression test that opens a loopback TCP session, then closes one end
 * while shutting down the other.  This triggers an unusual TCP stack case in
 * which an open file descriptor / socket is associated with a closed TCP
 * connection.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
tcp_server(pid_t partner, int listen_fd)
{
	int error, accept_fd;

	accept_fd = accept(listen_fd, NULL, NULL);
	if (accept_fd < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_server: accept");
	}
	close(accept_fd);
	close(listen_fd);
}

static void
tcp_client(pid_t partner, u_short port, int secs)
{
	struct sockaddr_in sin;
	int error, sock;

	sleep(1);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "socket");
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
	sin.sin_port = port;

	if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "connect");
	}

	if (shutdown(sock, SHUT_RDWR) < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "shutdown");
	}

	sleep(secs);
	close(sock);
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	pid_t child_pid, parent_pid;
	int listen_fd;
	socklen_t len;
	u_short port;

	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
		err(-1, "signal");

	/*
	 * Run the whole thing twice: once, with a short sleep in the client,
	 * so that we close before time wait runs out, and once with a long
	 * sleep so that the time wait terminates while the socket is open.
	 * We don't reuse listen sockets between runs.
	 */
	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err(-1, "socket");

	/*
	 * We use the loopback, but let the kernel select a port for the
	 * server socket.
	 */
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (bind(listen_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		err(-1, "bind");

	if (listen(listen_fd, -1) < 0)
		err(-1, "listen");

	/*
	 * Query the port so that the client can use it.
	 */
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	len = sizeof(sin);
	if (getsockname(listen_fd, (struct sockaddr *)&sin, &len) < 0)
		err(-1, "getsockname");
	port = sin.sin_port;
	printf("Using port %d\n", ntohs(port));

	parent_pid = getpid();
	child_pid = fork();
	if (child_pid < 0)
		err(-1, "fork");
	if (child_pid == 0) {
		child_pid = getpid();
		tcp_server(child_pid, listen_fd);
		exit(0);
	} else
		tcp_client(parent_pid, port, 1);
	(void)kill(child_pid, SIGTERM);
	close(listen_fd);
	sleep(5);

	/*
	 * Start again, this time long sleep.
	 */
	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0)
		err(-1, "socket");

	/*
	 * We use the loopback, but let the kernel select a port for the
	 * server socket.
	 */
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (bind(listen_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		err(-1, "bind");

	if (listen(listen_fd, -1) < 0)
		err(-1, "listen");

	/*
	 * Query the port so that the client can use it.
	 */
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	len = sizeof(sin);
	if (getsockname(listen_fd, (struct sockaddr *)&sin, &len) < 0)
		err(-1, "getsockname");
	port = sin.sin_port;
	printf("Using port %d\n", ntohs(port));

	parent_pid = getpid();
	child_pid = fork();
	if (child_pid < 0)
		err(-1, "fork");
	if (child_pid == 0) {
		child_pid = getpid();
		tcp_server(parent_pid, listen_fd);
	} else
		tcp_client(child_pid, port, 800);

	return (0);
}
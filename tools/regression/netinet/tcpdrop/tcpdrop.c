
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
 * TCP regression test for the tcpdrop sysctl; build a loopback TCP
 * connection, drop it, and make sure both endpoints return that the
 * connection has been reset.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
tcp_drop(struct sockaddr_in *sin_local, struct sockaddr_in *sin_remote)
{
	struct sockaddr_storage addrs[2];

	/*
	 * Sysctl accepts an array of two sockaddr's, the first being the
	 * 'foreign' sockaddr, the second being the 'local' sockaddr.
	 */

	bcopy(sin_remote, &addrs[0], sizeof(*sin_remote));
	bcopy(sin_local, &addrs[1], sizeof(*sin_local));

	return (sysctlbyname("net.inet.tcp.drop", NULL, 0, addrs,
	    sizeof(addrs)));
}

static void
tcp_server(pid_t partner, int listen_fd)
{
	int error, accept_fd;
	ssize_t len;
	char ch;

	accept_fd = accept(listen_fd, NULL, NULL);
	if (accept_fd < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_server: accept");
	}

	/*
	 * Send one byte, make sure that worked, wait for the drop, and try
	 * sending another.  By sending small amounts, we avoid blocking
	 * waiting on the remote buffer to be drained.
	 */
	ch = 'A';
	len = send(accept_fd, &ch, sizeof(ch), MSG_NOSIGNAL);
	if (len < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_server: send (1)");
	}
	if (len != sizeof(ch)) {
		(void)kill(partner, SIGTERM);
		errx(-1, "tcp_server: send (1) len");
	}

	sleep (10);

	ch = 'A';
	len = send(accept_fd, &ch, sizeof(ch), MSG_NOSIGNAL);
	if (len >= 0) {
		(void)kill(partner, SIGTERM);
		errx(-1, "tcp_server: send (2): success");
	} else if (errno != EPIPE) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_server: send (2)");
	}

	close(accept_fd);
	close(listen_fd);
}

static void
tcp_client(pid_t partner, u_short port)
{
	struct sockaddr_in sin, sin_local;
	int error, sock;
	socklen_t slen;
	ssize_t len;
	char ch;

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

	slen = sizeof(sin_local);
	if (getsockname(sock, (struct sockaddr *)&sin_local, &slen) < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "getsockname");
	}

	/*
	 * Send one byte, make sure that worked, wait for the drop, and try
	 * sending another.  By sending small amounts, we avoid blocking
	 * waiting on the remote buffer to be drained.
	 */
	ch = 'A';
	len = send(sock, &ch, sizeof(ch), MSG_NOSIGNAL);
	if (len < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_client: send (1)");
	}
	if (len != sizeof(ch)) {
		(void)kill(partner, SIGTERM);
		errx(-1, "tcp_client: send (1) len");
	}

	sleep(5);
	if (tcp_drop(&sin_local, &sin) < 0) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_client: tcp_drop");
	}
	sleep(5);

	ch = 'A';
	len = send(sock, &ch, sizeof(ch), MSG_NOSIGNAL);
	if (len >= 0) {
		(void)kill(partner, SIGTERM);
		errx(-1, "tcp_client: send (2): success");
	} else if (errno != EPIPE) {
		error = errno;
		(void)kill(partner, SIGTERM);
		errno = error;
		err(-1, "tcp_client: send (2)");
	}
	close(sock);
}

int
main(int argc, char *argv[])
{
	pid_t child_pid, parent_pid;
	struct sockaddr_in sin;
	int listen_fd;
	u_short port;
	socklen_t len;

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

	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
		err(-1, "signal");

	parent_pid = getpid();
	child_pid = fork();
	if (child_pid < 0)
		err(-1, "fork");
	if (child_pid == 0) {
		child_pid = getpid();
		tcp_server(parent_pid, listen_fd);
	} else
		tcp_client(child_pid, port);

	return (0);
}
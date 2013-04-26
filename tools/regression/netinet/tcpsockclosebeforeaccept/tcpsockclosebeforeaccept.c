
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
 * TCP regression test which opens a loopback TCP session, and closes it
 * before the remote endpoint (server) can accept it.  Run the test twice,
 * once using an explicit close() from the client, a second using a tcp drop.
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

#define	TCP_PORT	9005

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
tcp_server(pid_t partner)
{
	int error, listen_fd, accept_fd;
	struct sockaddr_in sin;

	listen_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "tcp_server: socket");
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = htons(TCP_PORT);

	if (bind(listen_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "tcp_server: bind");
	}

	if (listen(listen_fd, -1) < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "tcp_server: listen");
	}

	sleep(10);

	accept_fd = accept(listen_fd, NULL, NULL);
	if (accept_fd < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "tcp_server: accept");
	}
	close(accept_fd);
	close(listen_fd);
}

static void
tcp_client(pid_t partner, int dropflag)
{
	struct sockaddr_in sin, sin_local;
	int error, sock;
	socklen_t slen;

	sleep(1);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "socket");
	}

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
	sin.sin_port = htons(TCP_PORT);

	if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "connect");
	}

	slen = sizeof(sin_local);
	if (getsockname(sock, (struct sockaddr *)&sin_local, &slen) < 0) {
		error = errno;
		(void)kill(partner, SIGKILL);
		errno = error;
		err(-1, "getsockname");
	}

	if (dropflag) {
		if (tcp_drop(&sin_local, &sin) < 0) {
			error = errno;
			(void)kill(partner, SIGKILL);
			errno = error;
			err(-1, "tcp_drop");
		}
		sleep(2);
	}
	close(sock);
}

int
main(int argc, char *argv[])
{
	pid_t child_pid, parent_pid;

	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
		err(-1, "signal");

	parent_pid = getpid();
	child_pid = fork();
	if (child_pid < 0)
		err(-1, "fork");
	if (child_pid == 0) {
		child_pid = getpid();
		tcp_server(parent_pid);
		return (0);
	} else
		tcp_client(child_pid, 0);
	(void)kill(child_pid, SIGTERM);

	sleep(5);

	parent_pid = getpid();
	child_pid = fork();
	if (child_pid < 0)
		err(-1, "fork");
	if (child_pid == 0) {
		child_pid = getpid();
		tcp_server(parent_pid);
		return (0);
	} else
		tcp_client(child_pid, 1);
	(void)kill(child_pid, SIGTERM);

	return (0);
}
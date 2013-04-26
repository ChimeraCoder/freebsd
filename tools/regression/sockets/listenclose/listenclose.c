
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

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * The listenclose regression test is designed to catch kernel bugs that may
 * trigger as a result of performing a close on a listen() socket with as-yet
 * unaccepted connections in its queues.  This results in the connections
 * being aborted, which is a not-often-followed code path.  To do this, we
 * create a local TCP socket, build a non-blocking connection to it, and then
 * close the accept socket.  The connection must be non-blocking or the
 * program will block and as such connect() will not return as accept() is
 * never called.
 */

int
main(int argc, char *argv[])
{
	int listen_sock, connect_sock;
	struct sockaddr_in sin;
	socklen_t len;
	u_short port;
	int arg;

	listen_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_sock == -1)
		errx(-1,
		    "socket(PF_INET, SOCK_STREAM, 0) for listen socket: %s",
		    strerror(errno));


	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = 0;

	if (bind(listen_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errx(-1, "bind(%s, %d) for listen socket: %s",
		    inet_ntoa(sin.sin_addr), 0, strerror(errno));

	len = sizeof(sin);
	if (getsockname(listen_sock, (struct sockaddr *)&sin, &len) < 0)
		errx(-1, "getsockname() for listen socket: %s",
		    strerror(errno));
	port = sin.sin_port;

	if (listen(listen_sock, -1) < 0)
		errx(-1, "listen() for listen socket: %s", strerror(errno));

	connect_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (connect_sock == -1)
		errx(-1, "socket(PF_INET, SOCK_STREAM, 0) for connect "
		    "socket: %s", strerror(errno));

	arg = O_NONBLOCK;
	if (fcntl(connect_sock, F_SETFL, &arg) < 0)
		errx(-1, "socket(PF_INET, SOCK_STREAM, 0) for connect socket"
		    ": %s", strerror(errno));

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = port;

	if (connect(connect_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errx(-1, "connect() for connect socket: %s", strerror(errno));
	close(connect_sock);
	close(listen_sock);

	return (0);
}
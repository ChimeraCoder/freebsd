
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	struct sockaddr_in sock;
	socklen_t len;
	int listen_sock, connect_sock;
	u_short port;

	/* Shutdown(2) on an invalid file descriptor has to return EBADF. */
	if ((shutdown(listen_sock, SHUT_RDWR) != -1) && (errno != EBADF))
		errx(-1, "shutdown() for invalid file descriptor does not "
		    "return EBADF");

	listen_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_sock == -1)
		errx(-1,
		    "socket(PF_INET, SOCK_STREAM, 0) for listen socket: %s",
		    strerror(errno));

	bzero(&sock, sizeof(sock));
	sock.sin_len = sizeof(sock);
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sock.sin_port = 0;

	if (bind(listen_sock, (struct sockaddr *)&sock, sizeof(sock)) < 0)
		errx(-1, "bind(%s, %d) for listen socket: %s",
		    inet_ntoa(sock.sin_addr), sock.sin_port, strerror(errno));

	len = sizeof(sock);
	if (getsockname(listen_sock, (struct sockaddr *)&sock, &len) < 0)
		errx(-1, "getsockname() for listen socket: %s",
		    strerror(errno));
	port = sock.sin_port;

	if (listen(listen_sock, -1) < 0)
		errx(-1, "listen() for listen socket: %s", strerror(errno));

	connect_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (connect_sock == -1)
		errx(-1, "socket(PF_INET, SOCK_STREAM, 0) for connect "
		    "socket: %s", strerror(errno));

	bzero(&sock, sizeof(sock));
	sock.sin_len = sizeof(sock);
	sock.sin_family = AF_INET;
	sock.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sock.sin_port = port;

	if (connect(connect_sock, (struct sockaddr *)&sock, sizeof(sock)) < 0)
		errx(-1, "connect() for connect socket: %s", strerror(errno));
	/* Try to pass an invalid flags. */
	if ((shutdown(connect_sock, SHUT_RD - 1) != -1) && (errno != EINVAL))
		errx(-1, "shutdown(SHUT_RD - 1) does not return EINVAL");
	if ((shutdown(connect_sock, SHUT_RDWR + 1) != -1) && (errno != EINVAL))
		errx(-1, "shutdown(SHUT_RDWR + 1) does not return EINVAL");

	if (shutdown(connect_sock, SHUT_RD) < 0)
		errx(-1, "shutdown(SHUT_RD) for connect socket: %s",
		    strerror(errno));
	if (shutdown(connect_sock, SHUT_WR) < 0)
		errx(-1, "shutdown(SHUT_WR) for connect socket: %s",
		    strerror(errno));

	close(connect_sock);
	close(listen_sock);

	return (0);
}
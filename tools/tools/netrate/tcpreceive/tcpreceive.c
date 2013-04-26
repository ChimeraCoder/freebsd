
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
 * Back end to a variety of TCP-related benchmarks.  This program accepts TCP
 * connections on a port, and echoes all received data back to the sender.
 * It is capable of handling only one connection at a time out of a single
 * thread.
 */
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Simple micro-benchmark to see how many connections/second can be created
 * in a serialized fashion against a given server.  A timer signal is used
 * to interrupt the loop and assess the cost, and uses a fixed maximum
 * buffer size.  It makes no attempt to time out old connections.
 */
#define	BUFFERSIZE	128*1024
#define	PORT		6060

static void
handle_connection(int accept_sock)
{
	u_char buffer[BUFFERSIZE];
	ssize_t len, recvlen, sofar;
	int s;

	s = accept(accept_sock, NULL, NULL);
	if (s < 0) {
		warn("accept");
		return;
	}

	while (1) {
		recvlen = recv(s, buffer, BUFFERSIZE, 0);
		if (recvlen < 0 || recvlen == 0) {
			close(s);
			return;
		}
		sofar = 0;
		while (sofar < recvlen) {
			len = send(s, buffer + sofar, recvlen - sofar, 0);
			if (len < 0) {
				close(s);
				return;
			}
			sofar += len;
		}
	}
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	int accept_sock;

	accept_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (accept_sock < 0)
		err(-1, "socket(PF_INET, SOCKET_STREAM, 0)");

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_len = sizeof(sin);
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(PORT);

	if (bind(accept_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		err(-1, "bind");

	if (listen(accept_sock, -1) < 0)
		err(-1, "listen");

	while (1)
		handle_connection(accept_sock);

	return (0);
}
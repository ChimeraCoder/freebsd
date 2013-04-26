
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
 * Simple netipx regression test that attempts to build an SPX stream socket
 * pair, and send data twice over the stream, once in each direction.
 * Purposefully pick a small packet length that should fit into the buffers
 * at both ends, and therefore not result in a buffer deadlock.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netipx/ipx.h>

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define	IPX_ENDPOINT	"0xbebe.1.0x8a13"
#define	PACKETLEN	16 * (1024 * 1024)

static void
packet_fill(u_char *packet)
{
	int i;

	for (i = 0; i < PACKETLEN; i++)
		packet[i] = (i & 0xff);
}

static int
packet_check(u_char *packet, size_t totlen, ssize_t len)
{
	size_t i;

	for (i = totlen; i < totlen + len; i++) {
		if (packet[i] != (i & 0xff))
			return (-1);
	}
	return (0);
}

static void
my_send(int sock, const char *who, pid_t pid)
{
	u_char packet[PACKETLEN];
	ssize_t len;
	size_t totlen;
	int error;

	totlen = 0;
	packet_fill(packet);
	while (totlen < PACKETLEN) {
		len = send(sock, packet + totlen, PACKETLEN - totlen, 0);
		if (len < 0) {
			error = errno;
			(void)kill(pid, SIGTERM);
			errno = error;
			err(-1, "%s: send()", who);
		}
		if (len == 0) {
			(void)kill(pid, SIGTERM);
			errx(-1, "%s: send(): EOF", who);
		}
		totlen += len;
	}
}

static void
my_recv(int sock, const char *who, pid_t pid)
{
	u_char packet[PACKETLEN];
	ssize_t len;
	size_t totlen;
	int error;

	totlen = 0;
	bzero(packet, sizeof(packet));
	while (totlen < PACKETLEN) {
		len = recv(sock, packet + totlen, sizeof(packet) - totlen, 0);
		if (len < 0) {
			errno = error;
			(void)kill(pid, SIGTERM);
			errno = error;
			err(-1, "%s: recv()", who);
		}
		if (len == 0) {
			(void)kill(pid, SIGTERM);
			errx(-1, "%s: recv(): EOF", who);
		}
		if (packet_check(packet, totlen, len) < 0) {
			(void)kill(pid, SIGTERM);
			errx(-1, "%s: recv(): got bad data", who);
		}
		totlen += len;
	}
}

int
main(int argc, char *argv[])
{
	int error, sock_listen, sock_recv, sock_send;
	struct sockaddr_ipx sipx_listen, sipx_send;
	pid_t childpid, parentpid;

	/*
	 * Socket to receive with.
	 */
	sock_listen = socket(PF_IPX, SOCK_STREAM, 0);
	if (sock_listen < 0)
		err(-1, "sock_listen = socket(PF_IPX, SOCK_STREAM, 0)");

	bzero(&sipx_listen, sizeof(sipx_listen));
	sipx_listen.sipx_len = sizeof(sipx_listen);
	sipx_listen.sipx_family = AF_IPX;
	sipx_listen.sipx_addr = ipx_addr(IPX_ENDPOINT);

	if (bind(sock_listen, (struct sockaddr *)&sipx_listen,
	    sizeof(sipx_listen)) < 0)
		err(-1, "bind(sock_listen)");

	if (listen(sock_listen, -1) < 0)
		err(-1, "listen(sock_listen)");

	parentpid = getpid();

	childpid = fork();
	if (childpid < 0)
		err(-1, "fork()");

	if (childpid == 0) {
		/*
		 * The child: accept connections and process data on them.
		 */
		while (1) {
			sock_recv = accept(sock_listen, NULL, NULL);
			if (sock_recv < 0) {
				warn("accept()");
				continue;
			}

			my_recv(sock_recv, "listener", parentpid);
			my_send(sock_recv, "listener", parentpid);

			close(sock_recv);
		}
	} else {
		/*
		 * The parent: connect, send data, receive it back, and exit;
		 * build two connections, once using a full connect() API
		 * call, and the second using sendto().
		 */

		/*
		 * Socket to send with.
		 */
		sock_send = socket(PF_IPX, SOCK_STREAM, 0);
		if (sock_send < 0) {
			error = errno;
			(void)kill(childpid, SIGTERM);
			errno = error;
			err(-1, "sock_send = socket(PF_IPX, SOCK_STREAM, 0)");
		}

		bzero(&sipx_send, sizeof(sipx_send));
		sipx_send.sipx_len = sizeof(sipx_send);
		sipx_send.sipx_family = AF_IPX;
		sipx_send.sipx_addr = ipx_addr(IPX_ENDPOINT);

		if (connect(sock_send, (struct sockaddr *)&sipx_send,
		    sizeof(sipx_send)) < 0) {
			error = errno;
			(void)kill(childpid, SIGTERM);
			errno = error;
			err(-1, "sock_send = socket(PF_IPX, SOCK_STREAM, 0)");
		}

		my_send(sock_send, "connector", childpid);
		my_recv(sock_send, "connector", childpid);

		close(sock_send);

#ifdef SPX_SUPPORTS_SENDTO_WITH_CONNECT
		sock_send = socket(PF_IPX, SOCK_STREAM, 0);
		if (sock_send < 0) {
			error = errno;
			(void)kill(childpid, SIGTERM);
			errno = error;
			err(-1, "sock_send = socket(PF_IPX, SOCK_STREAM, 0)");
		}

		bzero(&sipx_send, sizeof(sipx_send));
		sipx_send.sipx_len = sizeof(sipx_send);
		sipx_send.sipx_family = AF_IPX;
		sipx_send.sipx_addr = ipx_addr(IPX_ENDPOINT);

		my_sendto(sock_send, "connector", childpid,
		    (struct sockaddr *)&sipx_send, sizeof(sipx_send));
		my_recv(sock_send, "connector", childpid);

		close(sock_send);
#endif

		(void)kill(childpid, SIGTERM);
	}

	return (0);
}
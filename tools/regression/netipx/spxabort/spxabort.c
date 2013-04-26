
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
 * Exercise the pru_abort() code for SPX by opening an SPX connection to a
 * listen socket, then closing the listen socket before accepting.
 *
 * We would also like to be able to test the other two abort cases, in which
 * incomplete connections are aborted due to overflow, and due to close of
 * the listen socket, but that requires a packet level test rather than using
 * the socket API.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netipx/ipx.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define	IPX_ENDPOINT	"0xbebe.1.0x8a13"

int
main(int argc, char *argv[])
{
	struct sockaddr_ipx sipx;
	int sock_listen, sock;

	sock_listen = socket(PF_IPX, SOCK_STREAM, 0);
	if (sock_listen < 0)
		err(-1, "sock_listen = socket(PF_IPX, SOCK_STREAM, 0)");

	bzero(&sipx, sizeof(sipx));
	sipx.sipx_len = sizeof(sipx);
	sipx.sipx_family = AF_IPX;
	sipx.sipx_addr = ipx_addr(IPX_ENDPOINT);

	if (bind(sock_listen, (struct sockaddr *)&sipx, sizeof(sipx)) < 0)
		err(-1, "bind(sock_listen)");

	if (listen(sock_listen, -1) < 0)
		err(-1, "listen(sock_listen)");

	sock = socket(PF_IPX, SOCK_STREAM, 0);
	if (sock < 0)
		err(-1, "sock = socket(PF_IPX, SOCK_STREAM, 0)");

	bzero(&sipx, sizeof(sipx));
	sipx.sipx_len = sizeof(sipx);
	sipx.sipx_family = AF_IPX;
	sipx.sipx_addr = ipx_addr(IPX_ENDPOINT);

	if (fcntl(sock, F_SETFL, O_NONBLOCK) < 0)
		err(-1, "fcntl(sock, F_SETFL, O_NONBLOCKING)");

	if (connect(sock, (struct sockaddr *)&sipx, sizeof(sipx)) < 0) {
		if (errno != EINPROGRESS)
			err(-1, "sock = socket(PF_IPX, SOCK_STREAM, 0)");
	}

	sleep(1);	/* Arbitrary. */

	close(sock_listen);

	return (0);
};
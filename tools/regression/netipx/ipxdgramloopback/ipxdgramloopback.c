
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
 * Simple netipx regression test that attempts to build an IPX datagram
 * socket pair and send a packet from one to the other.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netipx/ipx.h>

#include <err.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define	IPX_ENDPOINT	"0xbebe.1.0x8a13"
#define	PACKETLEN	128

int
main(int argc, char *argv[])
{
	struct sockaddr_ipx sipx_recv, sipx_send;
	u_char packet[PACKETLEN];
	int i, sock_recv, sock_send;
	ssize_t len;

	/*
	 * Socket to receive with.
	 */
	sock_recv = socket(PF_IPX, SOCK_DGRAM, 0);
	if (sock_recv < 0)
		err(-1, "sock_recv = socket(PF_IPX, SOCK_DGRAM, 0)");

	bzero(&sipx_recv, sizeof(sipx_recv));
	sipx_recv.sipx_len = sizeof(sipx_recv);
	sipx_recv.sipx_family = AF_IPX;
	sipx_recv.sipx_addr = ipx_addr(IPX_ENDPOINT);

	if (bind(sock_recv, (struct sockaddr *)&sipx_recv, sizeof(sipx_recv))
	    < 0)
		err(-1, "bind(sock_recv)");

	/*
	 * Set non-blocking to try to avoid blocking indefinitely if the
	 * packet doesn't end up in the right place.
	 */
	if (fcntl(sock_recv, F_SETFL, O_NONBLOCK) < 0)
		err(-1, "fcntl(O_NONBLOCK, sock_recv)");

	/*
	 * Socket to send with.
	 */
	sock_send = socket(PF_IPX, SOCK_DGRAM, 0);
	if (sock_send < 0)
		err(-1, "sock_send = socket(PF_IPX, SOCK_DGRAM, 0)");

	bzero(&sipx_send, sizeof(sipx_send));
	sipx_send.sipx_len = sizeof(sipx_send);
	sipx_send.sipx_family = AF_IPX;
	sipx_send.sipx_addr = ipx_addr(IPX_ENDPOINT);

	for (i = 0; i < PACKETLEN; i++)
		packet[i] = (i & 0xff);

	len = sendto(sock_send, packet, sizeof(packet), 0,
	    (struct sockaddr *)&sipx_send, sizeof(sipx_send));
	if (len < 0)
		err(-1, "sendto()");
	if (len != sizeof(packet))
		errx(-1, "sendto(): short send (%zu length, %zd sent)",
		    sizeof(packet), len);

	sleep(1);	/* Arbitrary non-zero amount. */

	bzero(packet, sizeof(packet));
	len = recv(sock_recv, packet, sizeof(packet), 0);
	if (len < 0)
		err(-1, "recv()");
	if (len != sizeof(packet))
		errx(-1, "recv(): short receive (%zu length, %zd received)",
		    sizeof(packet), len);

	for (i = 0; i < PACKETLEN; i++) {
		if (packet[i] != (i & 0xff))
			errx(-1, "recv(): byte %d wrong (%d instead of %d)",
			    i, packet[i], i & 0xff);
	}

	return (0);
}
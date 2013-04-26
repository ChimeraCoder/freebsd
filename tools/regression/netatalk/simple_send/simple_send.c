
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

#include <arpa/inet.h>

#include <netatalk/at.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * This is a simple test tool to bind netatalk SOCK_DGRAM sockets and perform
 * simple send operations that exercise each combination of bound and
 * connected endpoints, with the intent of exercising the various kernel send
 * case.
 *
 * In order to run this test, configure NETATALK into the kernel.  Use
 * ifconfig to set an appletalk address on an interface.  Run this tool with
 * two arguments: a local address and port number, and a remote address and
 * port number.
 *
 * It is recommended that you try running it with some interesting address
 * and port thresholds, including ATADDR_ANYNET, ATADDR_ANYNODE,
 * ATADDR_ANYPORT, and ATADDR_ANYBCAST.  Try both remote unicast addresses
 * and the local address, which will help to test local delivery (although
 * not socket receive).
 */

/*
 * Create a netatalk socket with specified source and destination, if
 * desired.  If a source is specified, bind it.  If a destination is
 * specified, connect it.
 */
static int
socket_between(struct sockaddr_at *from, struct sockaddr_at *to)
{
	int s;

	s = socket(PF_APPLETALK, SOCK_DGRAM, ATPROTO_DDP);
	if (s == -1)
		errx(1, "socket: %s\n", strerror(errno));

	if (from != NULL) {
		if (bind(s, (struct sockaddr *)from, sizeof(*from)) != 0)
			errx(1, "bind: %u.%u returned %s\n",
			    ntohs(from->sat_addr.s_net), from->sat_addr.s_node,
			    strerror(errno));
	}

	if (to != NULL) {
		if (connect(s, (struct sockaddr *)to, sizeof(*to)) != 0)
			errx(1, "connect: %u.%u returned %s\n",
			    ntohs(to->sat_addr.s_net), to->sat_addr.s_node,
			    strerror(errno));
	}
	return (s);
}

int
main(int argc, char *argv[])
{
	struct sockaddr_at sat_from, sat_to;
	char *addr_from, *addr_to;
	u_int net, node, port;
	char msg[] = "TEST";
	ssize_t len;
	int s;

	if (argc != 3)
		errx(1, "simple_send from_addr to_addr");

	addr_from = argv[1];
	sat_from.sat_family = AF_APPLETALK;
	sat_from.sat_len = sizeof(sat_from);
	if (sscanf(addr_from, "%u.%u:%u", &net, &node, &port) != 3 ||
	    net > 0xfff || node > 0xfe)
		errx(1, "%s: illegal address", addr_from);
	sat_from.sat_addr.s_net = htons(net);
	sat_from.sat_addr.s_node = node;
	sat_from.sat_port = port;

	addr_to = argv[2];
	sat_to.sat_family = AF_APPLETALK;
	sat_to.sat_len = sizeof(sat_to);
	if (sscanf(addr_to, "%u.%u:%u", &net, &node, &port) != 3 ||
	    net > 0xffff || node > 0xfe)
		errx(1, "%s: illegal address", addr_to);
	sat_to.sat_addr.s_net = htons(net);
	sat_to.sat_addr.s_node = node;
	sat_from.sat_port = port;

	printf("Address source is %u.%u:%u, address destination is %u.%u:%u\n",
	    ntohs(sat_from.sat_addr.s_net), sat_from.sat_addr.s_node,
	    sat_from.sat_port,
	    ntohs(sat_to.sat_addr.s_net), sat_to.sat_addr.s_node,
	    sat_to.sat_port);

	/*
	 * First, create a socket and use explicit sendto() to specify
	 * destination.
	 */
	s = socket_between(NULL, NULL);
	len = sendto(s, msg, sizeof(msg), 0, (struct sockaddr *)&sat_to,
	    sizeof(sat_to));
	close(s);

	/*
	 * Next, specify the destination for a connect() but not the source.
	 */
	s = socket_between(NULL, &sat_to);
	len = send(s, msg, sizeof(msg), 0);
	close(s);

	/*
	 * Now, bind the source, but not connect the destination.
	 */
	s = socket_between(&sat_from, NULL);
	len = sendto(s, msg, sizeof(msg), 0, (struct sockaddr *)&sat_to,
	    sizeof(sat_to));
	close(s);

	/*
	 * Finally, bind and connect.
	 */
	s = socket_between(&sat_from, &sat_to);
	len = send(s, msg, sizeof(msg), 0);
	close(s);

	exit(0);
}
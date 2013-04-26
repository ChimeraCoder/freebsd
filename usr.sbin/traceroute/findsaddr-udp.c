
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

#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include "findsaddr.h"
#include "traceroute.h"

/*
 * Return the source address for the given destination address.
 *
 * This makes use of proper source address selection in the FreeBSD kernel
 * even taking jails into account (sys/netinet/in_pcb.c:in_pcbladdr()).
 * We open a UDP socket, and connect to the destination, letting the kernel
 * do the bind and then read the source IPv4 address using getsockname(2).
 * This has multiple advantages: no need to do PF_ROUTE operations possibly
 * needing special privileges, jails properly taken into account and most
 * important - getting the result the kernel would give us rather than
 * best-guessing ourselves.
 */
const char *
findsaddr(register const struct sockaddr_in *to,
    register struct sockaddr_in *from)
{
	const char *errstr;
	struct sockaddr_in cto, cfrom;
	int s;
	socklen_t len;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s == -1)
		return ("failed to open DGRAM socket for src addr selection.");

	errstr = NULL;
	len = sizeof(struct sockaddr_in);
	memcpy(&cto, to, len);
	cto.sin_port = htons(65535);	/* Dummy port for connect(2). */
	if (connect(s, (struct sockaddr *)&cto, len) == -1) {
		errstr = "failed to connect to peer for src addr selection.";
		goto err;
	}

	if (getsockname(s, (struct sockaddr *)&cfrom, &len) == -1) {
		errstr = "failed to get socket name for src addr selection.";
		goto err;
	}

	if (len != sizeof(struct sockaddr_in) || cfrom.sin_family != AF_INET) {
		errstr = "unexpected address family in src addr selection.";
		goto err;
	}

	/* Update source address for traceroute. */
	setsin(from, cfrom.sin_addr.s_addr);

err:
	(void) close(s);

	/* No error (string) to return. */
	return (errstr);
}

/* end */
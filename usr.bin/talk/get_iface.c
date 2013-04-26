
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

/*
 * From:
 *  Id: find_interface.c,v 1.1 1995/08/14 16:08:39 wollman Exp
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "talk.h"

/*
 * Try to find the interface address that is used to route an IP
 * packet to a remote peer.
 */

int
get_iface(struct in_addr *dst, struct in_addr *iface)
{
	static struct sockaddr_in local;
	struct sockaddr_in remote;
	socklen_t namelen;
	int s, rv;

	memcpy(&remote.sin_addr, dst, sizeof remote.sin_addr);
	remote.sin_port = htons(60000);
	remote.sin_family = AF_INET;
	remote.sin_len = sizeof remote;

	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(60000);
	local.sin_family = AF_INET;
	local.sin_len = sizeof local;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		return -1;

	do {
		rv = bind(s, (struct sockaddr *)&local, sizeof local);
		local.sin_port = htons(ntohs(local.sin_port) + 1);
	} while(rv < 0 && errno == EADDRINUSE);

	if (rv < 0) {
		close(s);
		return -1;
	}

	do {
		rv = connect(s, (struct sockaddr *)&remote, sizeof remote);
		remote.sin_port = htons(ntohs(remote.sin_port) + 1);
	} while(rv < 0 && errno == EADDRINUSE);

	if (rv < 0) {
		close(s);
		return -1;
	}

	namelen = sizeof local;
	rv = getsockname(s, (struct sockaddr *)&local, &namelen);
	close(s);
	if (rv < 0)
		return -1;

	memcpy(iface, &local.sin_addr, sizeof local.sin_addr);
	return 0;
}
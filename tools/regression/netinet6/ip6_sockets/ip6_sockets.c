
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

#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#include <string.h>
#include <unistd.h>

/*
 * Simple regression test to create and close a variety of IPv6 socket types.
 */

int
main(int argc, char *argv[])
{
	struct sockaddr_in6 sin6;
	int s;

	/*
	 * UDPv6 simple test.
	 */
	s = socket(PF_INET6, SOCK_DGRAM, 0);
	if (s < 0)
		err(-1, "socket(PF_INET6, SOCK_DGRAM, 0)");
	close(s);

	/*
	 * UDPv6 connected case -- connect UDPv6 to an arbitrary port so that
	 * when we close the socket, it goes through the disconnect logic.
	 */
	s = socket(PF_INET6, SOCK_DGRAM, 0);
	if (s < 0)
		err(-1, "socket(PF_INET6, SOCK_DGRAM, 0)");
	bzero(&sin6, sizeof(sin6));
	sin6.sin6_len = sizeof(sin6);
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = in6addr_loopback;
	sin6.sin6_port = htons(1024);
	if (connect(s, (struct sockaddr *)&sin6, sizeof(sin6)) < 0)
		err(-1, "connect(SOCK_DGRAM, ::1)");
	close(s);

	/*
	 * TCPv6.
	 */
	s = socket(PF_INET6, SOCK_STREAM, 0);
	if (s < 0)
		err(-1, "socket(PF_INET6, SOCK_STREAM, 0)");
	close(s);

	/*
	 * Raw IPv6.
	 */
	s = socket(PF_INET6, SOCK_RAW, 0);
	if (s < 0)
		err(-1, "socket(PF_INET6, SOCK_RAW, 0)");
	close(s);

	return (0);
}
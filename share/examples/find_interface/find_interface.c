
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
 * This is a simple program which demonstrates how to query the kernel
 * routing mechanism using only a UDP socket.  Pass it a hostname on
 * the command line (sorry, it doesn't parse dotted decimal) and it will
 * print out an IP address which names the interface over which UDP
 * packets intended for that destination would be sent.
 * A more sophisticated program might use the list obtained from SIOCGIFCONF
 * to match the address with an interface name, but applications programmers
 * much more often need to know the address of the interface rather than
 * the name.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <sysexits.h>

int
main(int argc, char **argv)
{
	struct sockaddr_in local, remote;
	struct hostent *hp;
	int s, rv, namelen;

	argc--, argv++;

	if (!*argv) {
		errx(EX_USAGE, "must supply a hostname");
	}

	hp = gethostbyname(*argv);
	if (!hp) {
		errx(EX_NOHOST, "cannot resolve hostname: %s", *argv);
	}

	memcpy(&remote.sin_addr, hp->h_addr_list[0], sizeof remote.sin_addr);
	remote.sin_port = htons(60000);
	remote.sin_family = AF_INET;
	remote.sin_len = sizeof remote;

	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(60000);
	local.sin_family = AF_INET;
	local.sin_len = sizeof local;

	s = socket(PF_INET, SOCK_DGRAM, 0);
	if (s < 0)
		err(EX_OSERR, "socket");

	do {
		rv = bind(s, (struct sockaddr *)&local, sizeof local);
		local.sin_port = htons(ntohs(local.sin_port) + 1);
	} while(rv < 0 && errno == EADDRINUSE);

	if (rv < 0)
		err(EX_OSERR, "bind");

	do {
		rv = connect(s, (struct sockaddr *)&remote, sizeof remote);
		remote.sin_port = htons(ntohs(remote.sin_port) + 1);
	} while(rv < 0 && errno == EADDRINUSE);

	if (rv < 0)
		err(EX_OSERR, "connect");

	namelen = sizeof local;
	rv = getsockname(s, (struct sockaddr *)&local, &namelen);
	if (rv < 0)
		err(EX_OSERR, "getsockname");

	printf("Route to %s is out %s\n", *argv, inet_ntoa(local.sin_addr));
	return 0;
}
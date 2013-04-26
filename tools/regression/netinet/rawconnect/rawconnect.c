
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
 * Bug in IP code panics the system at close(2) on
 * connected SOCK_RAW, IPPROTO_IP socket.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

static void
usage(void)
{

	fprintf(stderr, "rawconnect: no arguments\n");
		exit(1);
}

int
main(int argc, __unused char *argv[])
{
	struct sockaddr_in sin;
	int sock;

	if (argc != 1)
		usage();

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_LOOPBACK;
	sin.sin_port = htons(65534);

	sock = socket(PF_INET, SOCK_RAW, IPPROTO_IP);
	if (sock == -1)
		err(1, "socket");

	if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		err(1, "connect");

	close(sock);

	return (0);
}
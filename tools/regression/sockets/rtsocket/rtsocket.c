
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
 * Simple routing socket regression test: create and destroy a raw routing
 * socket, and make sure that dgram and stream don't work, socketpair, etc.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <net/route.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	int sock, socks[2];

	sock = socket(PF_ROUTE, SOCK_STREAM, 0);
	if (sock >= 0) {
		close(sock);
		errx(-1, "socket(PF_ROUTE, SOCK_STREAM, 0) returned %d",
		    sock);
	}

	if (errno != EPROTONOSUPPORT)
		err(-1, "socket(PF_ROUTE, SOCK_STREAM, 0)");

	sock = socket(PF_ROUTE, SOCK_DGRAM, 0);
	if (sock >= 0) {
		close(sock);
		errx(-1, "socket(PF_ROUTE, SOCK_DGRAM, 0) returned %d",
		    sock);
	}

	if (errno != EPROTONOSUPPORT)
		err(-1, "socket(PF_ROUTE, SOCK_DGRAM, 0)");

	sock = socket(PF_ROUTE, SOCK_RAW, 0);
	if (sock < 0)
		err(-1, "socket(PF_ROUTE, SOCK_RAW, 0)");
	close(sock);

	if (socketpair(PF_ROUTE, SOCK_STREAM, 0, socks) == 0) {
		close(socks[0]);
		close(socks[1]);
		errx(-1,
		    "socketpair(PF_ROUTE, SOCK_STREAM, 0, socks) success");
	}

	if (errno != EPROTONOSUPPORT)
		err(-1, "socketpair(PF_ROUTE, SOCK_STREAM, 0, socks)");

	if (socketpair(PF_ROUTE, SOCK_DGRAM, 0, socks) == 0) {
		close(socks[0]);
		close(socks[1]);
		errx(-1,
		    "socketpair(PF_ROUTE, SOCK_DGRAM, 0, socks) success");
	}

	if (errno != EPROTONOSUPPORT)
		err(-1, "socketpair(PF_ROUTE, SOCK_DGRAM, 0, socks)");

	if (socketpair(PF_ROUTE, SOCK_RAW, 0, socks) == 0) {
		close(socks[0]);
		close(socks[1]);
		errx(-1,
		    "socketpair(PF_ROUTE, SOCK_STREAM, 0, socks) success");
	}

	return (0);
}
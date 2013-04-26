
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

#include <sys/param.h>
#include <sys/jail.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * A bug in the jail(8) code prevented processes in jail from properly
 * connecting UDP sockets.  This test program attempts to exercise that bug.
 */

static void
usage(void)
{

	fprintf(stderr, "udpconnectjail: no arguments\n");
	exit(-1);
}

static void
test(const char *context, struct sockaddr_in *sin)
{
	int sock;

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
		errx(-1, "%s: socket(PF_INET, SOCK_DGRAM, 0): %s", context,
		    strerror(errno));

	if (connect(sock, (struct sockaddr *)sin, sizeof(*sin)) < 0)
		errx(-1, "%s: connect(%s): %s", context,
		    inet_ntoa(sin->sin_addr), strerror(errno));

	if (close(sock) < 0)
		errx(-1, "%s: close(): %s", context, strerror(errno));
}

int
main(int argc, __unused char *argv[])
{
	struct sockaddr_in sin;
	struct jail thejail;
	struct in_addr ia4;

	if (argc != 1)
		usage();

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = htons(8080);	/* Arbitrary */

	/*
	 * First run the system call test outside of a jail.
	 */
	test("not in jail", &sin);

	/*
	 * Now re-run in a jail.
	 * XXX-BZ should switch to jail_set(2).
	 */
	ia4.s_addr = htonl(INADDR_LOOPBACK);

	bzero(&thejail, sizeof(thejail));
	thejail.version = JAIL_API_VERSION;
	thejail.path = "/";
	thejail.hostname = "jail";
	thejail.jailname = "udpconnectjail";
	thejail.ip4s = 1;
	thejail.ip4 = &ia4;
	
	if (jail(&thejail) < 0)
		errx(-1, "jail: %s", strerror(errno));
	test("in jail", &sin);

	fprintf(stdout, "PASS\n");

	return (0);
}

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
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * The UDP code allows transmitting zero-byte datagrams, but are they
 * received?
 */

#define	THEPORT	9543		/* Arbitrary. */

static void
usage(void)
{

	errx(-1, "no arguments allowed\n");
}

static void
test(int domain, const char *domainstr, struct sockaddr *sa, socklen_t salen)
{
	int sock_send, sock_receive;
	ssize_t size;

	sock_send = socket(domain, SOCK_DGRAM, 0);
	if (sock_send < 0)
		err(-1, "socket(%s, SOCK_DGRAM, 0)", domainstr);

	sock_receive = socket(domain, SOCK_DGRAM, 0);
	if (sock_receive < 0)
		err(-1, "socket(%s, SOCK_DGRAM, 0)", domainstr);

	if (bind(sock_receive, sa, salen) < 0)
		err(-1, "Protocol %s bind(sock_receive)", domainstr);
	if (fcntl(sock_receive, F_SETFL, O_NONBLOCK, 1) < 0)
		err(-1, "Protocll %s fcntl(sock_receive, FL_SETFL, "
		    "O_NONBLOCK)", domainstr);

	if (connect(sock_send, sa, salen) < 0)
		err(-1, "Protocol %s connect(sock_send)", domainstr);

	size = recv(sock_receive, NULL, 0, 0);
	if (size > 0)
		errx(-1, "Protocol %s recv(sock_receive, NULL, 0) before: %zd",
		    domainstr, size);
	else if (size < 0)
		err(-1, "Protocol %s recv(sock_receive, NULL, 0) before",
		    domainstr);

	size = send(sock_send, NULL, 0, 0);
	if (size < 0)
		err(-1, "Protocol %s send(sock_send, NULL, 0)", domainstr);

	(void)sleep(1);
	size = recv(sock_receive, NULL, 0, 0);
	if (size < 0)
		err(-1, "Protocol %s recv(sock_receive, NULL, 0) test",
		    domainstr);

	size = recv(sock_receive, NULL, 0, 0);
	if (size > 0)
		errx(-1, "Protocol %s recv(sock_receive, NULL, 0) after: %zd",
		    domainstr, size);
	else if (size < 0)
		err(-1, "Protocol %s recv(sock_receive, NULL, 0) after",
		    domainstr);
}

int
main(int argc, __unused char *argv[])
{
	struct sockaddr_un sun;
	struct sockaddr_in6 sin6;
	struct sockaddr_in sin;
	struct in6_addr loopback6addr = IN6ADDR_LOOPBACK_INIT;

	if (argc != 1)
		usage();

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_port = htons(THEPORT);

	test(PF_INET, "PF_INET", (struct sockaddr *)&sin, sizeof(sin));

	bzero(&sin6, sizeof(sin6));
	sin6.sin6_len = sizeof(sin6);
	sin6.sin6_family = AF_INET6;
	sin6.sin6_addr = loopback6addr;
	sin6.sin6_port = htons(THEPORT);

	test(PF_INET6, "PF_INET6", (struct sockaddr *)&sin6, sizeof(sin6));

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_LOCAL;
	strlcpy(sun.sun_path, "/tmp/udpzerosize-socket", sizeof(sun.sun_path));
	if (unlink(sun.sun_path) < 0 && errno != ENOENT)
		err(-1, "unlink: %s", sun.sun_path);

	test(PF_LOCAL, "PF_LOCAL", (struct sockaddr *)&sun, sizeof(sun));

	return (0);
}
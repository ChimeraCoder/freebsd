
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
 * This is a test tool for IP divert sockets.  For the time being, it just
 * exercise creation and binding of sockets, rather than their divert
 * behaviour.  It would be highly desirable to broaden this test tool to
 * include packet injection and diversion.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
ok(const char *test)
{

	fprintf(stderr, "%s: OK\n", test);
}

static void
fail(const char *test, const char *note)
{

	fprintf(stderr, "%s - %s: FAIL (%s)\n", test, note, strerror(errno));
	exit(1);
}

static void
failx(const char *test, const char *note)
{

	fprintf(stderr, "%s - %s: FAIL\n", test, note);
	exit(1);
}

static int
ipdivert_create(const char *test)
{
	int s;

	s = socket(PF_INET, SOCK_RAW, IPPROTO_DIVERT);
	if (s < 0)
		fail(test, "socket");
	return (s);
}

static void
ipdivert_close(const char *test, int s)
{

	if (close(s) < 0)
		fail(test, "close");
}

static void
ipdivert_bind(const char *test, int s, u_short port, int expect)
{
	struct sockaddr_in sin;
	int err;

	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port);

	err = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	if (err < 0) {
		if (expect == 0)
			fail(test, "bind");
		if (errno != expect)
			fail(test, "bind");
	} else {
		if (expect != 0)
			failx(test, "bind");
	}
}

int
main(int argc, char *argv[])
{
	const char *test;
	int s1, s2;

	/*
	 * First test: create and close an IP divert socket.
	 */
	test = "create_close";
	s1 = ipdivert_create(test);
	ipdivert_close(test, s1);
	ok(test);

	/*
	 * Second test: create, bind, and close an IP divert socket.
	 */
	test = "create_bind_close";
	s1 = ipdivert_create(test);
	ipdivert_bind(test, s1, 1000, 0);
	ipdivert_close(test, s1);
	ok(test);

	/*
	 * Third test: create two sockets, bind to different ports, and close.
	 * This should succeed due to non-conflict on the port numbers.
	 */
	test = "create2_bind2_close2";
	s1 = ipdivert_create(test);
	s2 = ipdivert_create(test);
	ipdivert_bind(test, s1, 1000, 0);
	ipdivert_bind(test, s2, 1001, 0);
	ipdivert_close(test, s1);
	ipdivert_close(test, s2);
	ok(test);

	/*
	 * Fourth test: create two sockets, bind to the *same* port, and
	 * close.  This should fail due to conflicting port numbers.
	 */
	test = "create2_bind2_conflict_close2";
	s1 = ipdivert_create(test);
	s2 = ipdivert_create(test);
	ipdivert_bind(test, s1, 1000, 0);
	ipdivert_bind(test, s2, 1000, EADDRINUSE);
	ipdivert_close(test, s1);
	ipdivert_close(test, s2);
	ok(test);

	return (0);
}
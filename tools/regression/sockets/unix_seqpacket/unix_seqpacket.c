
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

#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define	FAILERR(str)	err(-1, "%s: %s", __func__, str)
#define	FAILERRX(str)	errx(-1, "%s: %s", __func__, str)

static void
test_socket(void)
{
	int s;

	s = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (s < 0)
		FAILERR("socket");
	(void)close(s);
}

static void
test_socketpair(void)
{
	int sv[2];

	if (socketpair(PF_LOCAL, SOCK_SEQPACKET, 0, sv) < 0)
		FAILERR("socketpair");
	(void)close(sv[0]);
	(void)close(sv[1]);
}

static void
test_listen_unbound(void)
{
	int s;

	s = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (s < 0)
		FAILERR("socket");
	if (listen(s, -1) == 0)
		FAILERRX("listen");
	(void)close(s);
}

static void
test_bind(void)
{
	struct sockaddr_un sun;
	char path[PATH_MAX];
	int s;

	snprintf(path, sizeof(path), "/tmp/lds.XXXXXXXXX");
	if (mktemp(path) == NULL)
		FAILERR("mktemp");
	s = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (s < 0)
		FAILERR("socket");
	bzero(&sun, sizeof(sun));
	sun.sun_family = AF_LOCAL;
	sun.sun_len = sizeof(sun);
	strlcpy(sun.sun_path, path, sizeof(sun.sun_path));
	if (bind(s, (struct sockaddr *)&sun, sizeof(sun)) < 0)
		FAILERR("bind");
	close(s);
	(void)unlink(path);
}

static void
test_listen_bound(void)
{
	struct sockaddr_un sun;
	char path[PATH_MAX];
	int s;

	snprintf(path, sizeof(path), "/tmp/lds.XXXXXXXXX");
	if (mktemp(path) == NULL)
		FAILERR("mktemp");
	s = socket(PF_LOCAL, SOCK_SEQPACKET, 0);
	if (s < 0)
		FAILERR("socket");
	bzero(&sun, sizeof(sun));
	sun.sun_family = AF_LOCAL;
	sun.sun_len = sizeof(sun);
	strlcpy(sun.sun_path, path, sizeof(sun.sun_path));
	if (bind(s, (struct sockaddr *)&sun, sizeof(sun)) < 0)
		FAILERR("bind");
	if (listen(s, -1)) {
		(void)unlink(path);
		FAILERR("bind");
	}
	close(s);
	(void)unlink(path);
}

int
main(int argc, char *argv[])
{

	test_socket();
	test_socketpair();
	test_listen_unbound();
	test_bind();
	test_listen_bound();
}
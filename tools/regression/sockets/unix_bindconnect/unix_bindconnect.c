
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * Simple regression test to exercise some error cases relating to the use of
 * bind() and connect() on UNIX domain sockets.  In particular, make sure
 * that when two sockets rendezvous using the file system name space, they
 * get the expected success/failure cases.
 *
 * TODO:
 * - Check that the resulting file mode/owner are right.
 * - Do the same tests with UNIX domain sockets.
 * - Check the results of getsockaddr() and getpeeraddr().
 */

#define	SOCK_NAME_ONE	"socket.1"
#define	SOCK_NAME_TWO	"socket.2"

#define	UNWIND_MAX	1024

int unwind_len;
struct unwind {
	char	u_path[PATH_MAX];
} unwind_list[UNWIND_MAX];

static void
push_path(const char *path)
{

	if (unwind_len >= UNWIND_MAX)
		err(-1, "push_path: one path too many (%s)", path);

	strlcpy(unwind_list[unwind_len].u_path, path, PATH_MAX);
	unwind_len++;
}

static void
unwind(void)
{
	int i;

	for (i = unwind_len - 1; i >= 0; i--) {
		unlink(unwind_list[i].u_path);
		rmdir(unwind_list[i].u_path);
	}
}

static int
bind_test(const char *directory_path)
{
	char socket_path[PATH_MAX];
	struct sockaddr_un sun;
	int sock1, sock2;

	sock1 = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock1 < 0) {
		warn("bind_test: socket(PF_UNIX, SOCK_STREAM, 0)");
		return (-1);
	}

	if (snprintf(socket_path, sizeof(socket_path), "%s/%s",
	    directory_path, SOCK_NAME_ONE) >= PATH_MAX) {
		warn("bind_test: snprintf(socket_path)");
		close(sock1);
		return (-1);
	}

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	if (snprintf(sun.sun_path, sizeof(sun.sun_path), "%s", socket_path)
	    >= sizeof(sun.sun_path)) {
		warn("bind_test: snprintf(sun.sun_path)");
		close(sock1);
		return (-1);
	}

	if (bind(sock1, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
		warn("bind_test: bind(sun) #1");
		close(sock1);
		return (-1);
	}

	push_path(socket_path);

	/*
	 * Once a STREAM UNIX domain socket has been bound, it can't be
	 * rebound.  Expected error is EINVAL.
	 */
	if (bind(sock1, (struct sockaddr *)&sun, sizeof(sun)) == 0) {
		warnx("bind_test: bind(sun) #2 succeeded");
		close(sock1);
		return (-1);
	}
	if (errno != EINVAL) {
		warn("bind_test: bind(sun) #2");
		close(sock1);
		return (-1);
	}

	sock2 = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock2 < 0) {
		warn("bind_test: socket(PF_UNIX, SOCK_STREAM, 0)");
		close(sock1);
		return (-1);
	}

	/*
	 * Since a socket is already bound to the pathname, it can't be bound
	 * to a second socket.  Expected error is EADDRINUSE.
	 */
	if (bind(sock2, (struct sockaddr *)&sun, sizeof(sun)) == 0) {
		warnx("bind_test: bind(sun) #3 succeeded");
		close(sock1);
		close(sock2);
		return (-1);
	}
	if (errno != EADDRINUSE) {
		warn("bind_test: bind(sun) #2");
		close(sock1);
		close(sock2);
		return (-1);
	}

	close(sock1);

	/*
	 * The socket bound to the pathname has been closed, but the pathname
	 * can't be reused without first being unlinked.  Expected error is
	 * EADDRINUSE.
	 */
	if (bind(sock2, (struct sockaddr *)&sun, sizeof(sun)) == 0) {
		warnx("bind_test: bind(sun) #4 succeeded");
		close(sock2);
		return (-1);
	}
	if (errno != EADDRINUSE) {
		warn("bind_test: bind(sun) #4");
		close(sock2);
		return (-1);
	}

	unlink(socket_path);

	/*
	 * The pathname is now free, so the socket should be able to bind to
	 * it.
	 */
	if (bind(sock2, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
		warn("bind_test: bind(sun) #5");
		close(sock2);
		return (-1);
	}

	close(sock2);
	return (0);
}

static int
connect_test(const char *directory_path)
{
	char socket_path[PATH_MAX];
	struct sockaddr_un sun;
	int sock1, sock2;

	sock1 = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock1 < 0) {
		warn("connect_test: socket(PF_UNIX, SOCK_STREAM, 0)");
		return (-1);
	}

	if (snprintf(socket_path, sizeof(socket_path), "%s/%s",
	    directory_path, SOCK_NAME_TWO) >= PATH_MAX) {
		warn("connect_test: snprintf(socket_path)");
		close(sock1);
		return (-1);
	}

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	if (snprintf(sun.sun_path, sizeof(sun.sun_path), "%s", socket_path)
	    >= sizeof(sun.sun_path)) {
		warn("connect_test: snprintf(sun.sun_path)");
		close(sock1);
		return (-1);
	}

	/*
	 * Try connecting to a path that doesn't yet exist.  Should fail with
	 * ENOENT.
	 */
	if (connect(sock1, (struct sockaddr *)&sun, sizeof(sun)) == 0) {
		warnx("connect_test: connect(sun) #1 succeeded");
		close(sock1);
		return (-1);
	}
	if (errno != ENOENT) {
		warn("connect_test: connect(sun) #1");
		close(sock1);
		return (-1);
	}

	if (bind(sock1, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
		warn("connect_test: bind(sun) #1");
		close(sock1);
		return (-1);
	}

	if (listen(sock1, 3) < 0) {
		warn("connect_test: listen(sock1)");
		close(sock1);
		return (-1);
	}

	push_path(socket_path);

	sock2 = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock2 < 0) {
		warn("socket(PF_UNIX, SOCK_STREAM, 0)");
		close(sock1);
		return (-1);
	}

	/*
	 * Do a simple connect and make sure that works.
	 */
	if (connect(sock2, (struct sockaddr *)&sun, sizeof(sun)) < 0) {
		warn("connect(sun) #2");
		close(sock1);
		return (-1);
	}

	close(sock2);

	close(sock1);

	sock2 = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock2 < 0) {
		warn("socket(PF_UNIX, SOCK_STREAM, 0)");
		return (-1);
	}

	/*
	 * Confirm that once the listen socket is closed, we get a
	 * connection refused (ECONNREFUSED) when attempting to connect to
	 * the pathname.
	 */
	if (connect(sock2, (struct sockaddr *)&sun, sizeof(sun)) == 0) {
		warnx("connect(sun) #3 succeeded");
		close(sock2);
		return (-1);
	}
	if (errno != ECONNREFUSED) {
		warn("connect(sun) #3");
		close(sock2);
		return (-1);
	}

	close(sock2);
	unlink(socket_path);
	return (0);
}
int
main(int argc, char *argv[])
{
	char directory_path[PATH_MAX];
	int error;

	strlcpy(directory_path, "/tmp/unix_bind.XXXXXXX", PATH_MAX);
	if (mkdtemp(directory_path) == NULL)
		err(-1, "mkdtemp");
	push_path(directory_path);

	error = bind_test(directory_path);

	if (error == 0)
		error = connect_test(directory_path);

	unwind();
	return (error);
}
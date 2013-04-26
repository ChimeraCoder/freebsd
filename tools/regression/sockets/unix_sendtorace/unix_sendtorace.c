
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
 * Attempts to exercise UNIX domain socket races relating to the non-atomic
 * connect-and-send properties of sendto().  As the result of such a race is
 * a kernel panic, this test simply completes or doesn't.
 *
 * XXX: Despite implementing support for sendto() on stream sockets with
 * implied connect, the appropriate flag isn't set in the FreeBSD kernel so
 * it does not work.  For now, don't call the stream test.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#define	PATH		"/tmp/123"
#define	ITERATIONS	1000000

static void
stream_server(int listenfd)
{
	int acceptfd;

	while (1) {
		acceptfd = accept(listenfd, NULL, NULL);
		if (acceptfd < 0) {
			warn("stream_server: accept");
			continue;
		}
		sleep(1);
		close(acceptfd);
	}
}

static void
stream_client(void)
{
	struct sockaddr_un sun;
	ssize_t len;
	char c = 0;
	int fd, i;

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, PATH);
	for (i = 0; i < ITERATIONS; i++) {
		fd = socket(PF_UNIX, SOCK_STREAM, 0);
		if (fd < 0) {
			warn("stream_client: socket");
			return;
		}
		len = sendto(fd, &c, sizeof(c), 0, (struct sockaddr *)&sun,
		    sizeof(sun));
		if (len < 0)
			warn("stream_client: sendto");
		close(fd);
	}
}

static void
stream_test(void)
{
	struct sockaddr_un sun;
	pid_t childpid;
	int listenfd;

	listenfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (listenfd < 0)
		err(-1, "stream_test: socket");

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, PATH);

	if (bind(listenfd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
		err(-1, "stream_test: bind");

	if (listen(listenfd, -1) < 0)
		err(-1, "stream_test: listen");

	childpid = fork();
	if (childpid < 0)
		err(-1, "stream_test: fork");

	if (childpid != 0) {
		sleep(1);
		stream_client();
		kill(childpid, SIGTERM);
		sleep(1);
	} else
		stream_server(listenfd);

	(void)unlink(PATH);
}

static void
datagram_server(int serverfd)
{
	ssize_t len;
	char c;

	while (1) {
		len = recv(serverfd, &c, sizeof(c), 0);
		if (len < 0)
			warn("datagram_server: recv");
	}
}

static void
datagram_client(void)
{
	struct sockaddr_un sun;
	ssize_t len;
	char c = 0;
	int fd, i;

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, PATH);
	for (i = 0; i < ITERATIONS; i++) {
		fd = socket(PF_UNIX, SOCK_DGRAM, 0);
		if (fd < 0) {
			warn("datagram_client: socket");
			return;
		}
		len = sendto(fd, &c, sizeof(c), 0, (struct sockaddr *)&sun,
		    sizeof(sun));
		if (len < 0)
			warn("datagram_client: sendto");
		close(fd);
	}
}

static void
datagram_test(void)
{
	struct sockaddr_un sun;
	pid_t childpid;
	int serverfd;

	serverfd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (serverfd < 0)
		err(-1, "datagram_test: socket");

	bzero(&sun, sizeof(sun));
	sun.sun_len = sizeof(sun);
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, PATH);

	if (bind(serverfd, (struct sockaddr *)&sun, sizeof(sun)) < 0)
		err(-1, "datagram_test: bind");

	childpid = fork();
	if (childpid < 0)
		err(-1, "datagram_test: fork");

	if (childpid != 0) {
		sleep(1);
		datagram_client();
		kill(childpid, SIGTERM);
		sleep(1);
	} else
		datagram_server(serverfd);

	(void)unlink(PATH);
}

int
main(int argc, char *argv[])
{

	(void)unlink(PATH);
	datagram_test();
	if (0)
		stream_test();
	return (0);
}
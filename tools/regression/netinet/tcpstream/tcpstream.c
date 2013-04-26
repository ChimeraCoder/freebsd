
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
 * tcpstream sets up a simple TCP client and server, and then streams a
 * predictable pseudo-random byte sequence through it using variable block
 * sizes.  The intent is to to detect corruption of data in the TCP stream.
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define	MAX_LOOPS	10240
#define	MAX_LONGS	1024

static void
usage(void)
{

	fprintf(stderr, "tcpstream client [ip] [port] [seed]\n");
	fprintf(stderr, "tcpstream server [port] [seed]\n");
	exit(-1);
}

static void
fill_buffer(long *buffer, int len)
{
	int i;

	for (i = 0; i < len; i++)
		buffer[i] = htonl(random());
}

static int
check_buffer(long *buffer, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (buffer[i] != htonl(random()))
			return (0);
	}
	return (1);
}

static void
tcpstream_client(struct sockaddr_in sin, long seed)
{
	long buffer[MAX_LONGS];
	ssize_t len;
	int i, j, sock;

	srandom(seed);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
		errx(-1, "socket: %s", strerror(errno));

	if (connect(sock, (struct sockaddr *) &sin, sizeof(sin)) == -1)
		errx(-1, "connect: %s", strerror(errno));

	for (j = 0; j < MAX_LOOPS; j++) {
		for (i = 0; i < MAX_LONGS; i++) {
			fill_buffer(buffer, i);
			len = send(sock, buffer, i * sizeof(long), 0);
			if (len == -1) {
				printf("%d bytes written of %d expected\n",
				    len, i * sizeof(long));
				fflush(stdout);
				perror("send");
				goto done;
			}
		}
	}

done:
	close(sock);
}

static void
tcpstream_server(struct sockaddr_in sin, long seed)
{
	int i, j, listen_sock, accept_sock;
	struct sockaddr_in other_sin;
	long buffer[MAX_LONGS];
	socklen_t addrlen;
	ssize_t len;

	int input_byte_counter;

	listen_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_sock == -1)
		errx(-1, "socket: %s", strerror(errno));

	if (bind(listen_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		errx(-1, "bind: %s", strerror(errno));

	if (listen(listen_sock, -1) == -1)
		errx(-1, "listen: %s", strerror(errno));

	while (1) {
		bzero(&other_sin, sizeof(other_sin));
		addrlen = sizeof(other_sin);

		accept_sock = accept(listen_sock, (struct sockaddr *)
		    &other_sin, &addrlen);
		if (accept_sock == -1) {
			perror("accept");
			continue;
		}
		printf("connection opened from %s:%d\n",
		    inet_ntoa(other_sin.sin_addr), ntohs(other_sin.sin_port));
		input_byte_counter = 0;

		srandom(seed);

		for (j = 0; j < MAX_LOOPS; j++) {
			for (i = 0; i < MAX_LONGS; i++) {
				len = recv(accept_sock, buffer,
				    i * sizeof(long), MSG_WAITALL);
				if (len != i * sizeof(long)) {
					perror("recv");
					goto done;
				}
				if (check_buffer(buffer, i) == 0) {
					fprintf(stderr,
    "Corruption in block beginning %d and ending %d\n", input_byte_counter,
    input_byte_counter + len);
					fprintf(stderr,
    "Block size %d\n", i * sizeof(long));
					goto done;
				}
				input_byte_counter += len;
			}
		}
done:
		printf("connection closed\n");
		close(accept_sock);
	}
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	long port, seed;
	char *dummy;

	if (argc < 2)
		usage();
	if (strcmp(argv[1], "client") == 0) {
		if (argc != 5)
			usage();

		bzero(&sin, sizeof(sin));
		sin.sin_len = sizeof(sin);
		sin.sin_family = AF_INET;

		if (inet_aton(argv[2], &sin.sin_addr) != 1)
			errx(-1, "%s: %s", argv[2], strerror(EINVAL));

		port = strtoul(argv[3], &dummy, 10);
		if (port < 1 || port > 65535 || *dummy != '\0')
			usage();
		sin.sin_port = htons(port);

		seed = strtoul(argv[4], &dummy, 10);
		if (*dummy != '\0')
			usage();

		tcpstream_client(sin, seed);

	} else if (strcmp(argv[1], "server") == 0) {
		if (argc != 4)
			usage();

		bzero(&sin, sizeof(sin));
		sin.sin_len = sizeof(sin);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;

		port = strtoul(argv[2], &dummy, 10);
		if (port < 1 || port > 65535 || *dummy != '\0')
			usage();
		sin.sin_port = htons(port);

		seed = strtoul(argv[3], &dummy, 10);
		if (*dummy != '\0')
			usage();

		tcpstream_server(sin, seed);
	} else
		usage();

	return (0);
}
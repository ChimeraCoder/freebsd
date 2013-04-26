
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

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static void
usage(void)
{

	fprintf(stderr, "tcpconnect server port\n");
	fprintf(stderr, "tcpconnect client ip port count [nonblock] [tcpmd5]\n");
	exit(-1);
}

static void
tcpconnect_server(int argc, char *argv[])
{
	int listen_sock, accept_sock;
	struct sockaddr_in sin;
	char *dummy;
	long port;

	if (argc != 1)
		usage();

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	port = strtoul(argv[0], &dummy, 10);
	if (port < 1 || port > 65535 || *dummy != '\0')
		usage();
	sin.sin_port = htons(port);

	listen_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_sock == -1)
		err(-1, "socket");

	if (bind(listen_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		err(-1, "bind");

	if (listen(listen_sock, -1) == -1)
		err(-1, "listen");

	while (1) {
		accept_sock = accept(listen_sock, NULL, NULL);
		close(accept_sock);
	}
}

static void
tcpconnect_client(int argc, char *argv[])
{
	struct sockaddr_in sin;
	long count, i, port;
	char *dummy;
	int sock;
	int nonblock = 0, md5enable = 0;

	if (argc < 3 || argc > 5)
		usage();
	for (i=3; i < argc; i++) {
		if (strcmp(argv[i], "nonblock") == 0)
			nonblock = 1;
		if (strcmp(argv[i], "tcpmd5") == 0)
			md5enable = 1;
	}

	bzero(&sin, sizeof(sin));
	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	if (inet_aton(argv[0], &sin.sin_addr) == 0)
		err(-1, "listen");

	port = strtoul(argv[1], &dummy, 10);
	if (port < 1 || port > 65535 || *dummy != '\0')
		usage();
	sin.sin_port = htons(port);

	count = strtoul(argv[2], &dummy, 10);
	if (count < 1 || count > 100000 || *dummy != '\0')
		usage();

	for (i = 0; i < count; i++) {
		sock = socket(PF_INET, SOCK_STREAM, 0);
		if (sock == -1)
			err(-1, "socket");

		/* No warning in default case on ENOPROTOOPT. */
		if (setsockopt(sock, IPPROTO_TCP, TCP_MD5SIG,
		    &md5enable, sizeof(md5enable)) != 0) {
			if (errno == ENOPROTOOPT && md5enable > 0)
				err(-1, "setsockopt(TCP_MD5SIG)");
			else if (errno != ENOPROTOOPT)
				warn("setsockopt(TCP_MD5SIG)");
		}

		if (nonblock) {
			if (fcntl(sock, F_SETFL, O_NONBLOCK) != 0)
				err(-1, "fcntl(F_SETFL)");

			if (connect(sock, (struct sockaddr *)&sin,
			    sizeof(sin)) == -1 && errno != EINPROGRESS)
				err(-1, "connect");
		} else {
			if (connect(sock, (struct sockaddr *)&sin,
			    sizeof(sin)) == -1)
				err(-1, "connect");
		}

		close(sock);
	}
}

int
main(int argc, char *argv[])
{

	if (argc < 2)
		usage();

	if (strcmp(argv[1], "server") == 0)
		tcpconnect_server(argc - 2, argv + 2);
	else if (strcmp(argv[1], "client") == 0)
		tcpconnect_client(argc - 2, argv + 2);
	else
		usage();

	exit(0);
}
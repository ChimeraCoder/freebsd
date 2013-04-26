
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
 * This program uses select() to check that someone is trying to connect
 * before calling accept().
 */
main()
{
	int sock, length;
	struct sockaddr_in server;
	int msgsock;
	char buf[1024];
	int rval;
	fd_set ready;
	struct timeval to;

	/* Create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("opening stream socket");
		exit(1);
	}
	/* Name socket using wildcards */
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;
	if (bind(sock, &server, sizeof(server))) {
		perror("binding stream socket");
		exit(1);
	}
	/* Find out assigned port number and print it out */
	length = sizeof(server);
	if (getsockname(sock, &server, &length)) {
		perror("getting socket name");
		exit(1);
	}
	printf("Socket has port #%d\en", ntohs(server.sin_port));

	/* Start accepting connections */
	listen(sock, 5);
	do {
		FD_ZERO(&ready);
		FD_SET(sock, &ready);
		to.tv_sec = 5;
		if (select(sock + 1, &ready, 0, 0, &to) < 0) {
			perror("select");
			continue;
		}
		if (FD_ISSET(sock, &ready)) {
			msgsock = accept(sock, (struct sockaddr *)0, (int *)0);
			if (msgsock == -1)
				perror("accept");
			else do {
				bzero(buf, sizeof(buf));
				if ((rval = read(msgsock, buf, 1024)) < 0)
					perror("reading stream message");
				else if (rval == 0)
					printf("Ending connection\en");
				else
					printf("-->%s\en", buf);
			} while (rval > 0);
			close(msgsock);
		} else
			printf("Do something else\en");
	} while (TRUE);
}
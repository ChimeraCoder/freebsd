
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
 * Test for the non-blocking big pipe bug (write(2) returning
 * EAGAIN while select(2) returns the descriptor as ready for write).
 *
 * $FreeBSD$
 */
void write_frame(int fd, char *buf, unsigned long buflen)
{
    fd_set wfd;
    int i;

    while (buflen) {
	FD_ZERO(&wfd);
	FD_SET(fd, &wfd);
	i = select(fd+1, NULL, &wfd, NULL, NULL);
	if (i < 0) {
	    perror("select");
	    exit(1);
	}
	if (i != 1) {
	    fprintf(stderr, "select returned unexpected value %d\n", i);
	    exit(1);
	}
        i = write(fd, buf, buflen);
	if (i < 0) {
	    if (errno != EAGAIN)
		perror("write");
	    exit(1);
	}
        buf += i;
        buflen -= i;
    }
}

int main()
{
    char buf[BIG_PIPE_SIZE];	/* any value over PIPE_SIZE should do */
    int i, flags, fd[2];

    printf("1..1\n");

    if (pipe(fd) < 0) { perror("pipe"); exit(1); }

    flags = fcntl(fd[1], F_GETFL);
    if (flags == -1 || fcntl(fd[1], F_SETFL, flags|O_NONBLOCK) == -1) {
	perror("fcntl");
	exit(1);
    }

    switch (fork()) {
	case -1:
	    perror("fork");
	    exit(1);
	case 0:
	    close(fd[1]);
	    for (;;) {
		i = read(fd[0], buf, 256); /* any small size should do */
		if (i == 0) break;
		if (i < 0) { perror("read"); exit(1); }
	    }
	    exit(0);
	default:
	    break;
    }

    close(fd[0]);
    memset(buf, 0, sizeof buf);
    for (i = 0; i < 1000; i++) write_frame(fd[1], buf, sizeof buf);
    printf("ok 1\n");
    exit(0);
}

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
 * Regression test for uiomove in kernel; specifically for PR kern/38495.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>

#define TEST_SOCKET "/tmp/test_socket"

static jmp_buf myjmpbuf;

void handle_sigalrm(int signo);

void handle_sigalrm(int signo)
{
	longjmp(myjmpbuf, 1);
}

int
main(int argc, char *argv[])
{
	struct sockaddr_un un;
	pid_t pid;
	int s;

	s = socket(PF_LOCAL, SOCK_DGRAM, 0);
	if (s == -1)
		errx(-1, "socket");
	memset(&un, 0, sizeof(un));
	un.sun_family = AF_LOCAL;
	unlink(TEST_SOCKET);
	strcpy(un.sun_path, TEST_SOCKET);
	if (bind(s, (struct sockaddr *)&un, sizeof(un)) == -1)
		errx(-1, "bind");
	pid = fork();
	if (pid == -1)
		errx(-1, "fork");
	if (pid == 0) {
		int conn;
		char buf[] = "AAAAAAAAA";

		close(s);
		conn = socket(AF_LOCAL, SOCK_DGRAM, 0);
		if (conn == -1)
			errx(-1,"socket");
		if (sendto(conn, buf, sizeof(buf), 0, (struct sockaddr *)&un,
		    sizeof(un)) != sizeof(buf))
			errx(-1,"sendto");
		close(conn);
		_exit(0);
	}

	sleep(5);

	/* Make sure the data is there when we try to receive it. */
	if (recvfrom(s, (void *)-1, 1, 0, NULL, NULL) != -1)
		errx(-1,"recvfrom succeeded when failure expected");

	(void)signal(SIGALRM, handle_sigalrm);
	if (setjmp(myjmpbuf) == 0) {
		/*
	 	 * This recvfrom will panic an unpatched system, and block
		 * a patched one.
		 */
		alarm(5);
		(void)recvfrom(s, (void *)-1, 1, 0, NULL, NULL);
	}

	/* We should reach here via longjmp() and all should be well. */

	return (0);
}
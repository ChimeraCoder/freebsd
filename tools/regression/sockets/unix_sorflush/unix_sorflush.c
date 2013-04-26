
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
 * Reproduce a race in which:
 *
 * - Process (a) is blocked in read on a socket waiting on data.
 * - Process (b) is blocked in shutdown() on a socket waiting on (a).
 * - Process (c) delivers a signal to (b) interrupting its wait.
 *
 * This race is premised on shutdown() not interrupting (a) properly, and the
 * signal to (b) causing problems in the kernel.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/socket.h>

#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void
receive_and_exit(int s)
{
	ssize_t ssize;
	char ch;

	ssize = recv(s, &ch, sizeof(ch), 0);
	if (ssize < 0)
		err(-1, "receive_and_exit: recv");
	exit(0);
}

static void
shutdown_and_exit(int s)
{

	if (shutdown(s, SHUT_RD) < 0)
		err(-1, "shutdown_and_exit: shutdown");
	exit(0);
}

int
main(int argc, char *argv[])
{
	pid_t pida, pidb;
	int sv[2];

	if (socketpair(PF_LOCAL, SOCK_STREAM, 0, sv) < 0)
		err(-1, "socketpair");

	pida = fork();
	if (pida < 0)
		err(-1, "fork");
	if (pida == 0)
		receive_and_exit(sv[1]);
	sleep(1);
	pidb = fork();
	if (pidb < 0) {
		warn("fork");
		(void)kill(pida, SIGKILL);
		exit(-1);
	}
	if (pidb == 0)
		shutdown_and_exit(sv[1]);
	sleep(1);
	if (kill(pidb, SIGKILL) < 0)
		err(-1, "kill");
	sleep(1);
	printf("ok 1 - unix_sorflush\n");
	exit(0);
}

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
 * This regression test attempts to trigger a race that occurs when both
 * endpoints of a connected UNIX domain socket are closed at once.  The two
 * close paths may run concurrently leading to a call to sodisconnect() on an
 * already-closed socket in kernel.  Before it was fixed, this might lead to
 * ENOTCONN being returned improperly from close().
 *
 * This race is fairly timing-dependent, so it effectively requires SMP, and
 * may not even trigger then.
 */

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#define	UNIXSTR_PATH	"/tmp/mytest.socket"
#define	USLEEP	100
#define	LOOPS	100000

int
main(int argc, char **argv)
{
	struct sockaddr_un servaddr;
	int listenfd, connfd, pid;
	u_int counter, ncpus;
	size_t len;

	len = sizeof(ncpus);
	if (sysctlbyname("kern.smp.cpus", &ncpus, &len, NULL, 0) < 0)
		err(1, "kern.smp.cpus");
	if (len != sizeof(ncpus))
		errx(1, "kern.smp.cpus: invalid length");
	if (ncpus < 2)
		warnx("SMP not present, test may be unable to trigger race");

	/*
	 * Create a UNIX domain socket that the child will repeatedly
	 * accept() from, and that the parent will repeatedly connect() to.
	 */
	if ((listenfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
		err(1, "parent: socket error");
	(void)unlink(UNIXSTR_PATH);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, UNIXSTR_PATH);
	if (bind(listenfd, (struct sockaddr *) &servaddr,
	    sizeof(servaddr)) < 0)
		err(1, "parent: bind error");
	if (listen(listenfd, 1024) < 0)
		err(1, "parent: listen error");

	pid = fork();
	if (pid == -1)
		err(1, "fork()");
	if (pid != 0) {
		/*
		 * In the parent, repeatedly connect and disconnect from the
		 * socket, attempting to induce the race.
		 */
		close(listenfd);
		sleep(1);
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sun_family = AF_LOCAL;
		strcpy(servaddr.sun_path, UNIXSTR_PATH);
		for (counter = 0; counter < LOOPS; counter++) {
			if ((connfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
				(void)kill(pid, SIGTERM);
				err(1, "parent: socket error");
			}
			if (connect(connfd, (struct sockaddr *)&servaddr,
			    sizeof(servaddr)) < 0) {
			    	(void)kill(pid, SIGTERM);
				err(1, "parent: connect error");
			}
			if (close(connfd) < 0) {
				(void)kill(pid, SIGTERM);
				err(1, "parent: close error");
			}
			usleep(USLEEP);
		}
		(void)kill(pid, SIGTERM);
	} else {
		/*
		 * In the child, loop accepting and closing.  We may pick up
		 * the race here so report errors from close().
		 */
		for ( ; ; ) {
			if ((connfd = accept(listenfd,
			    (struct sockaddr *)NULL, NULL)) < 0)
				err(1, "child: accept error");
			if (close(connfd) < 0)
				err(1, "child: close error");
		}
	}
	printf("OK\n");
	exit(0);
}
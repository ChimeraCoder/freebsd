
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
__FBSDID("$FreeBSD$");

#include <sys/wait.h>
#include <sys/ioctl.h>

#include <dev/filemon/filemon.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>

/*
 * This simple test of filemon expects a test script called
 * "test_script.sh" in the cwd.
 */

int
main(void) {
	char log_name[] = "filemon_log.XXXXXX";
	pid_t child;
	int fm_fd, fm_log;

	if ((fm_fd = open("/dev/filemon", O_RDWR)) == -1)
		err(1, "open(\"/dev/filemon\", O_RDWR)");
	if ((fm_log = mkstemp(log_name)) == -1)
		err(1, "mkstemp(%s)", log_name);

	if (ioctl(fm_fd, FILEMON_SET_FD, &fm_log) == -1)
		err(1, "Cannot set filemon log file descriptor");

	/* Set up these two fd's to close on exec. */
	(void)fcntl(fm_fd, F_SETFD, FD_CLOEXEC);
	(void)fcntl(fm_log, F_SETFD, FD_CLOEXEC);

	switch (child = fork()) {
	case 0:
		child = getpid();
		if (ioctl(fm_fd, FILEMON_SET_PID, &child) == -1)
			err(1, "Cannot set filemon PID to %d", child);
		system("./test_script.sh");
		break;
	case -1:
		err(1, "Cannot fork");
	default:
		wait(&child);
		close(fm_fd);
//		printf("Results in %s\n", log_name);
		break;
	}
	return 0;
}
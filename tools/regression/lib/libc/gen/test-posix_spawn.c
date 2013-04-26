
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
 * Test program for posix_spawn() and posix_spawnp() as specified by
 * IEEE Std. 1003.1-2008.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/wait.h>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spawn.h>

int
main(int argc, char *argv[])
{
	int error, status;
	pid_t pid, waitres;
	char *myargs[4];
	char *myenv[2] = { "answer=42", NULL };

	/* Make sure we have no child processes. */
	while (waitpid(-1, NULL, 0) != -1)
		;
	assert(errno == ECHILD);

	/* Simple test. */
	myargs[0] = "sh";
	myargs[1] = "-c";
	myargs[2] = "exit $answer";
	myargs[3] = NULL;
	error = posix_spawnp(&pid, myargs[0], NULL, NULL, myargs, myenv);
	assert(error == 0);
	waitres = waitpid(pid, &status, 0);
	assert(waitres == pid);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 42);

	/*
	 * If the executable does not exist, the function shall either fail
	 * and not create a child process or succeed and create a child
	 * process that exits with status 127.
	 */
	myargs[0] = "/var/empty/nonexistent";
	myargs[1] = NULL;
	error = posix_spawn(&pid, myargs[0], NULL, NULL, myargs, myenv);
	if (error == 0) {
		waitres = waitpid(pid, &status, 0);
		assert(waitres == pid);
		assert(WIFEXITED(status) && WEXITSTATUS(status) == 127);
	} else {
		assert(error == ENOENT);
		waitres = waitpid(-1, NULL, 0);
		assert(waitres == -1 && errno == ECHILD);
	}

	printf("PASS posix_spawn()\n");
	printf("PASS posix_spawnp()\n");

	return (0);
}
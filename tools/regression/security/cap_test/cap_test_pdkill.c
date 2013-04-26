
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
 * Test routines to make sure a variety of system calls are or are not
 * available in capability mode.  The goal is not to see if they work, just
 * whether or not they return the expected ECAPMODE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <sys/capability.h>
#include <sys/errno.h>
#include <sys/procdesc.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <err.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>

#include "cap_test.h"

void handle_signal(int);
void handle_signal(int sig) {
	exit(PASSED);
}

int
test_pdkill(void)
{
	int success = PASSED;
	int pd, error;
	pid_t pid;

	//cap_enter();

	error = pdfork(&pd, 0);
	if (error < 0)
		err(-1, "pdfork");

	else if (error == 0) {
		signal(SIGINT, handle_signal);
		sleep(3600);
		exit(FAILED);
	}

	/* Parent process; find the child's PID (we'll need it later). */
	error = pdgetpid(pd, &pid);
	if (error != 0)
		FAIL("pdgetpid");

	/* Kill the child! */
	usleep(100);
	error = pdkill(pd, SIGINT);
	if (error != 0)
		FAIL("pdkill");

	/* Make sure the child finished properly. */
	int status;
	while (waitpid(pid, &status, 0) != pid) {}
	if ((success == PASSED) && WIFEXITED(status))
		success = WEXITSTATUS(status);
	else
		success = FAILED;

	return (success);
}
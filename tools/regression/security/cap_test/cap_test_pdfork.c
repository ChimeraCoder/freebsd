
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
#include <sys/stat.h>
#include <sys/wait.h>

#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <time.h>

#include "cap_test.h"

int
test_pdfork(void)
{
	struct stat stat;
	int success = PASSED;
	int pd, error;
	pid_t pid;
	time_t now;

	//cap_enter();

	pid = pdfork(&pd, 0);
	if (pid < 0)
		err(-1, "pdfork");

	else if (pid == 0) {
		/*
		 * Child process.
		 *
		 * pd should not be a valid process descriptor.
		 */
		error = pdgetpid(pd, &pid);
		if (error != -1)
			FAILX("pdgetpid succeeded");
		else if (errno != EBADF)
			FAIL("pdgetpid failed, but errno != EBADF");

		exit(success);
	}

	/* Parent process. Ensure that [acm]times have been set correctly. */
	REQUIRE(fstat(pd, &stat));

	now = time(NULL);
	CHECK(now != (time_t)-1);

	CHECK(now >= stat.st_birthtime);
	CHECK((now - stat.st_birthtime) < 2);
	CHECK(stat.st_birthtime == stat.st_atime);
	CHECK(stat.st_atime == stat.st_ctime);
	CHECK(stat.st_ctime == stat.st_mtime);

	/* Wait for the child to finish. */
	error = pdgetpid(pd, &pid);
	CHECK(error == 0);
	CHECK(pid > 0);

	int status;
	while (waitpid(pid, &status, 0) != pid) {}
	if ((success == PASSED) && WIFEXITED(status))
		success = WEXITSTATUS(status);
	else
		success = FAILED;

	return (success);
}

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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/capability.h>
#include <sys/procdesc.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "misc.h"

int
main(void)
{
	unsigned int mode;
	pid_t pid;
	int pfd;

	printf("1..27\n");

	mode = 666;
	CHECK(cap_getmode(&mode) == 0);
	/* If cap_getmode() succeeded mode should be modified. */
	CHECK(mode != 666);
	/* We are not in capability mode. */
	CHECK(mode == 0);

	/* Expect EFAULT. */
	errno = 0;
	CHECK(cap_getmode(NULL) == -1);
	CHECK(errno == EFAULT);
	errno = 0;
	CHECK(cap_getmode((void *)(uintptr_t)0xdeadc0de) == -1);
	CHECK(errno == EFAULT);

	/* If parent is not in capability mode, child after fork() also won't be. */
	pid = fork();
	switch (pid) {
	case -1:
		err(1, "fork() failed");
	case 0:
		mode = 666;
		CHECK(cap_getmode(&mode) == 0);
		/* If cap_getmode() succeeded mode should be modified. */
		CHECK(mode != 666);
		/* We are not in capability mode. */
		CHECK(mode == 0);
		exit(0);
	default:
		if (waitpid(pid, NULL, 0) == -1)
			err(1, "waitpid() failed");
	}

	/* If parent is not in capability mode, child after pdfork() also won't be. */
	pid = pdfork(&pfd, 0);
	switch (pid) {
	case -1:
		err(1, "pdfork() failed");
	case 0:
		mode = 666;
		CHECK(cap_getmode(&mode) == 0);
		/* If cap_getmode() succeeded mode should be modified. */
		CHECK(mode != 666);
		/* We are not in capability mode. */
		CHECK(mode == 0);
		exit(0);
	default:
		if (pdwait(pfd) == -1)
			err(1, "pdwait() failed");
		close(pfd);
	}

	/* In capability mode... */

	CHECK(cap_enter() == 0);

	mode = 666;
	CHECK(cap_getmode(&mode) == 0);
	/* If cap_getmode() succeeded mode should be modified. */
	CHECK(mode != 666);
	/* We are in capability mode. */
	CHECK(mode == 1);

	/* Expect EFAULT. */
	errno = 0;
	CHECK(cap_getmode(NULL) == -1);
	CHECK(errno == EFAULT);
	errno = 0;
	CHECK(cap_getmode((void *)(uintptr_t)0xdeadc0de) == -1);
	CHECK(errno == EFAULT);

	/* If parent is in capability mode, child after fork() also will be. */
	pid = fork();
	switch (pid) {
	case -1:
		err(1, "fork() failed");
	case 0:
		mode = 666;
		CHECK(cap_getmode(&mode) == 0);
		/* If cap_getmode() succeeded mode should be modified. */
		CHECK(mode != 666);
		/* We are in capability mode. */
		CHECK(mode == 1);
		exit(0);
	default:
		/*
		 * wait(2) and friends are not permitted in the capability mode,
		 * so we can only just wait for a while.
		 */
		sleep(1);
	}

	/* If parent is in capability mode, child after pdfork() also will be. */
	pid = pdfork(&pfd, 0);
	switch (pid) {
	case -1:
		err(1, "pdfork() failed");
	case 0:
		mode = 666;
		CHECK(cap_getmode(&mode) == 0);
		/* If cap_getmode() succeeded mode should be modified. */
		CHECK(mode != 666);
		/* We are in capability mode. */
		CHECK(mode == 1);
		exit(0);
	default:
		if (pdwait(pfd) == -1)
			err(1, "pdwait() failed");
		close(pfd);
	}

	exit(0);
}
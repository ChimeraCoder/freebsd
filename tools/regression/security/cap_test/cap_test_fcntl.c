
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
 * Test that fcntl works in capability mode.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/capability.h>
#include <sys/errno.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/wait.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cap_test.h"

/* A filename->descriptor mapping. */
struct fd {
	char	*f_name;
	int	 f_fd;
};

/*
 * Ensure that fcntl() works consistently for both regular file descriptors and
 * capability-wrapped ones.
 */
int
test_fcntl(void)
{
	int success = PASSED;
	cap_rights_t rights = CAP_READ | CAP_FCNTL;

	/*
	 * Open some files of different types, and wrap them in capabilities.
	 */
	struct fd files[] = {
		{ "file",         open("/etc/passwd", O_RDONLY) },
		{ "socket",       socket(PF_LOCAL, SOCK_STREAM, 0) },
		{ "SHM",          shm_open(SHM_ANON, O_RDWR, 0600) },
	};
	REQUIRE(files[0].f_fd);
	REQUIRE(files[1].f_fd);
	REQUIRE(files[2].f_fd);

	struct fd caps[] = {
		{ "file cap",     cap_new(files[0].f_fd, rights) },
		{ "socket cap",   cap_new(files[1].f_fd, rights) },
		{ "SHM cap",      cap_new(files[2].f_fd, rights) },
	};
	REQUIRE(caps[0].f_fd);
	REQUIRE(caps[1].f_fd);
	REQUIRE(caps[2].f_fd);

	struct fd all[] = {
		files[0], caps[0],
		files[1], caps[1],
		files[2], caps[2],
	};
	const size_t len = sizeof(all) / sizeof(struct fd);

	REQUIRE(cap_enter());

	/*
	 * Ensure that we can fcntl() all the files that we opened above.
	 */
	for (size_t i = 0; i < len; i++)
	{
		struct fd f = all[i];
		int cap;

		CHECK_SYSCALL_SUCCEEDS(fcntl, f.f_fd, F_GETFL, 0);
		REQUIRE(cap = cap_new(f.f_fd, CAP_READ));
		if (fcntl(f.f_fd, F_GETFL, 0) == -1)
			FAIL("Error calling fcntl('%s', F_GETFL)", f.f_name);
		else
			CHECK_NOTCAPABLE(fcntl, cap, F_GETFL, 0);
	}

	return (success);
}
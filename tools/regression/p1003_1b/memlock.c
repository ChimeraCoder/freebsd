
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
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>

#include "prutil.h"

int memlock(int argc, char *argv[])
{
	int e = 0;

	/* Is memory locking configured?
	 */
	errno = 0;
	if (sysconf(_SC_MEMLOCK) == -1) {
		if (errno != 0) {
			/* This isn't valid - may be a standard violation
			 */
			quit("(should not happen) sysconf(_SC_MEMLOCK)");
		}
		else {
			fprintf(stderr,
			"Memory locking is not supported in this environment.\n");
			e = -1;
		}
	}

	/* Lock yourself in memory:
	 */
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
		perror("mlockall(MCL_CURRENT | MCL_FUTURE)");
		e = errno;
	}
	else if (munlockall() == -1) {
		perror("munlockall");
		e = errno;
	}

	return e;
}

#ifdef NO_MEMLOCK
int mlockall(int flags)
{
	return EOPNOTSUPP;
}

int munlockall(void)
{
	return EOPNOTSUPP;
}
	

#endif

#ifdef STANDALONE_TESTS
int main(int argc, char *argv[]) { return memlock(argc, argv); }
#endif
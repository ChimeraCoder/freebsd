
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
 * Test that various sysctls are (and aren't) available on capability mode.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/capability.h>
#include <sys/errno.h>
#include <sys/sysctl.h>
#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cap_test.h"

/*
 * Certain sysctls are permitted in capability mode, but most are not.  Test
 * for the ones that should be, and try one or two that shouldn't.
 */
int
test_sysctl(void)
{
	int i, oid[2];
	int success = PASSED;
	size_t len;

	oid[0] = CTL_KERN;
	oid[1] = KERN_OSRELDATE;
	len = sizeof(i);
	CHECK(sysctl(oid, 2, &i, &len, NULL, 0) == 0);

	return (success);
}

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
 * Test that privilege is required to write to the system extended attribute
 * namespace.
 */

#include <sys/types.h>
#include <sys/extattr.h>

#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

#define	EA_NAMESPACE	EXTATTR_NAMESPACE_SYSTEM
#define	EA_NAME		"test"
#define	EA_DATA		"test"
#define	EA_SIZE		strlen(EA_DATA)

static char fpath[1024];
static int fpath_initialized;

int
priv_vfs_extattr_system_setup(int asroot, int injail, struct test *test)
{

	/*
	 * Set file perms so that discretionary access control would grant
	 * write rights on non-system EAs on the file.
	 */
	setup_file("priv_vfs_extattr_system_setup: fpath", fpath, UID_ROOT,
	    GID_WHEEL, 0666);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_extattr_system(int asroot, int injail, struct test *test)
{
	ssize_t ret;
	int error;

	ret = extattr_set_file(fpath, EA_NAMESPACE, EA_NAME, EA_DATA,
	    EA_SIZE);
	if (ret < 0)
		error = -1;
	else if (ret == EA_SIZE)
		error = 0;
	else
		err(-1, "priv_vfs_extattr_system: set returned %zd", ret);
	if (asroot && injail)
		expect("priv_vfs_extattr_system(asroot, injail)", error, -1,
		    EPERM);
	if (asroot && !injail)
		expect("priv_vfs_extattr_system(asroot, !injail)", error, 0,
		    0);
	if (!asroot && injail)
		expect("priv_vfs_extattr_system(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_extattr_system(!asroot, !injail)", error,
		    -1, EPERM);
}

void
priv_vfs_extattr_system_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}
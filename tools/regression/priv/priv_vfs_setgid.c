
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
 * Test that privilege is required to set the sgid bit on a file with a group
 * that isn't in the process credential.  The file uid owner is set to the
 * uid being tested with, as we are not interested in testing privileges
 * associated with file ownership.
 */

#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;

int
priv_vfs_setgid_fowner_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_setgid_fowner: fpath", fpath, UID_OWNER,
	    GID_OWNER, 0600);
	fpath_initialized = 1;
	return (0);
}

int
priv_vfs_setgid_fother_setup(int asroot, int injail, struct test *test)
{

	/* NOTE: owner uid, *other* gid. */
	setup_file("priv_vfs_setgid_forther: fpath", fpath, UID_OWNER,
	    GID_OTHER, 0600);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_setgid_fowner(int asroot, int injail, struct test *test)
{
	int error;

	error = chmod(fpath, 0600 | S_ISGID);
	if (asroot && injail)
		expect("priv_vfs_setgid_fowner(asroot, injail)", error, 0,
		    0);
	if (asroot && !injail)
		expect("priv_vfs_setgid_fowner(asroot, !injail)", error, 0,
		    0);
	if (!asroot && injail)
		expect("priv_vfs_setgid_fowner(!asroot, injail)", error, 0,
		    0);
	if (!asroot && !injail)
		expect("priv_vfs_setgid_fowner(!asroot, !injail)", error, 0,
		    0);
}

void
priv_vfs_setgid_fother(int asroot, int injail, struct test *test)
{
	int error;

	error = chmod(fpath, 0600 | S_ISGID);
	if (asroot && injail)
		expect("priv_vfs_setgid_fother(asroot, injail)", error, 0,
		    0);
	if (asroot && !injail)
		expect("priv_vfs_setgid_fother(asroot, !injail)", error, 0,
		    0);
	if (!asroot && injail)
		expect("priv_vfs_setgid_fother(!asroot, injail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_setgid_fother(!asroot, !injail)", error, -1,
		    EPERM);
}

void
priv_vfs_setgid_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}

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
 * Confirm that privilege is required in the cases using chown():
 *
 * - If the process euid does not match the file uid.
 *
 * - If the target uid is different than the current uid.
 *
 * - If the target gid changes and we the process is not a member of the new
 *   group.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;

/*
 * Check that changing the uid of a file requires privilege.
 */
int
priv_vfs_chown_uid_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_chown_uid: fpath", fpath, UID_ROOT, GID_WHEEL,
	    0600);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_chown_uid(int asroot, int injail, struct test *test)
{
	int error;

	error = chown(fpath, UID_OWNER, -1);
	if (asroot && injail)
		expect("priv_vfs_chown_uid(root, jail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chown_uid(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chown_uid(!root, jail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chown_uid(!root, !jail)", error, -1, EPERM);
}

/*
 * Check that changing the gid of a file owned by the user is allowed without
 * privilege as long as the gid matches the process.
 */
int
priv_vfs_chown_mygid_setup(int asroot, int injail, struct test *test)
{

	/*
	 * Create a file with a matching uid to the test process, but not a
	 * matching gid.
	 */
	setup_file("priv_vfs_chown_mygid: fpath", fpath, asroot ? UID_ROOT :
	    UID_OWNER, GID_OTHER, 0600);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_chown_mygid(int asroot, int injail, struct test *test)
{
	int error;

	error = chown(fpath, -1, asroot ? GID_WHEEL : GID_OWNER);
	if (asroot && injail)
		expect("priv_vfs_chown_mygid(root, jail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chown_mygid(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chown_mygid(!root, !jail)", error, 0, 0);
	if (!asroot && !injail)
		expect("priv_vfs_chown_mygid(!root, !jail)", error, 0, 0);
}

/*
 * Check that changing the gid of a file owned by the user is not allowed
 * without privilege if the gid doesn't match the process.
 */
int
priv_vfs_chown_othergid_setup(int asroot, int injail, struct test *test)
{

	/*
	 * Create a file with a matching uid to the test process with a
	 * matching gid.
	 */
	setup_file("priv_vfs_chown_othergid: fpath", fpath, asroot ? UID_ROOT
	    : UID_OWNER, asroot ? GID_WHEEL : GID_OWNER, 0600);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_chown_othergid(int asroot, int injail, struct test *test)
{
	int error;

	error = chown(fpath, -1, GID_OTHER);
	if (asroot && injail)
		expect("priv_vfs_chown_othergid(root, jail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chown_othergid(root, !jail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chown_othergid(!root, !jail)", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chown_othergid(!root, !jail)", error, -1,
		    EPERM);
}

void
priv_vfs_chown_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}
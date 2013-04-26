
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
 * Check that privilege is required to set the sticky bit on a file but not a
 * directory.
 */

#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

char fpath[1024];
int fpath_initialized;

char dpath[1024];
int dpath_initialized;

int
priv_vfs_stickyfile_dir_fowner_setup(int asroot, int injail,
    struct test *test)
{

	setup_dir("priv_vfs_stickyfile_fowner_setup: dpath", dpath,
	    UID_OWNER, GID_OWNER, 0700);
	dpath_initialized = 1;
	return (0);
}

int
priv_vfs_stickyfile_dir_fother_setup(int asroot, int injail,
    struct test *test)
{

	setup_dir("priv_vfs_stickyfile_fother_setup: dpath", dpath,
	    UID_OTHER, GID_OTHER, 0700);
	dpath_initialized = 1;
	return (0);
}

int
priv_vfs_stickyfile_file_fowner_setup(int asroot, int injail,
    struct test *test)
{

	setup_file("priv_vfs_stickyfile_fowner_setup: fpath", fpath,
	    UID_OWNER, GID_OWNER, 0600);
	fpath_initialized = 1;
	return (0);
}

int
priv_vfs_stickyfile_file_fother_setup(int asroot, int injail,
    struct test *test)
{

	setup_file("priv_vfs_stickyfile_fother_setup: fpath", fpath,
	    UID_OTHER, GID_OTHER, 0600);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_stickyfile_dir_fowner(int asroot, int injail, struct test *test)
{
	int error;

	error = chmod(dpath, 0700 | S_ISTXT);
	if (asroot && injail)
		expect("priv_vfs_stickyfile_dir_fowner(root, jail)", error,
		    0, 0);
	if (asroot && !injail)
		expect("priv_vfs_stickyfile_dir_fowner(root, !jail)", error,
		    0, 0);
	if (!asroot && injail)
		expect("priv_vfs_stickyfile_dir_fowner(!root, jail)", error,
		    0, 0);
	if (!asroot && !injail)
		expect("priv_vfs_stickyfile_dir_fowner(!root, !jail)", error,
		    0, 0);
}

void
priv_vfs_stickyfile_dir_fother(int asroot, int injail, struct test *test)
{
	int error;

	error = chmod(dpath, 0700 | S_ISTXT);
	if (asroot && injail)
		expect("priv_vfs_stickyfile_dir_fother(root, jail)", error,
		    0, 0);
	if (asroot && !injail)
		expect("priv_vfs_stickyfile_dir_fother(root, !jail)", error,
		    0, 0);
	if (!asroot && injail)
		expect("priv_vfs_stickyfile_dir_fother(!root, jail)", error,
		    -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_stickyfile_dir_fother(!root, !jail)", error,
		    -1, EPERM);
}

void
priv_vfs_stickyfile_file_fowner(int asroot, int injail, struct test *test)
{
	int error;

	error = chmod(fpath, 0600 | S_ISTXT);
	if (asroot && injail)
		expect("priv_vfs_stickyfile_file_fowner(root, jail)", error,
		    0, 0);
	if (asroot && !injail)
		expect("priv_vfs_stickyfile_file_fowner(root, !jail)", error,
		    0, 0);
	if (!asroot && injail)
		expect("priv_vfs_stickyfile_file_fowner(!root, jail)", error,
		    -1, EFTYPE);
	if (!asroot && !injail)
		expect("priv_vfs_stickyfile_file_fowner(!root, !jail)", error,
		    -1, EFTYPE);
}

void
priv_vfs_stickyfile_file_fother(int asroot, int injail, struct test *test)
{
	int error;

	error = chmod(fpath, 0600 | S_ISTXT);
	if (asroot && injail)
		expect("priv_vfs_stickyfile_file_fother(root, jail)", error,
		    0, 0);
	if (asroot && !injail)
		expect("priv_vfs_stickyfile_file_fother(root, !jail)", error,
		    0, 0);
	if (!asroot && injail)
		expect("priv_vfs_stickyfile_file_fother(!root, jail)", error,
		    -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_stickyfile_file_fother(!root, !jail)", error,
		    -1, EPERM);
}

void
priv_vfs_stickyfile_dir_cleanup(int asroot, int injail, struct test *test)
{

	if (dpath_initialized) {
		(void)rmdir(dpath);
		dpath_initialized = 0;
	}
}

void
priv_vfs_stickyfile_file_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}
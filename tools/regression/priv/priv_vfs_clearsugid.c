
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
 * There are three cases in which the file system will clear the setuid or
 * setgid bits on a file when running unprivileged:
 *
 * - When the file is chown()'d and either of the uid or the gid is changed.
 *   (currently, only changing the file gid applies, as privilege is required
 *   to change the uid).
 *
 * - The file is written to successfully.
 *
 * - An extended attribute of the file is written to successfully.
 *
 * In each case, check that the flags are cleared if unprivileged, and that
 * they aren't cleared if privileged.
 *
 * We can't use expect() as we're looking for side-effects rather than
 * success/failure of the system call.
 */

#include <sys/types.h>
#include <sys/extattr.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;

/*
 * If running as root, check that SUID is still set; otherwise, check that it
 * is not.
 */
static void
confirm_sugid(char *test_case, int asroot, int injail)
{
	struct stat sb;

	if (stat(fpath, &sb) < 0) {
		warn("%s stat(%s)", test_case, fpath);
		return;
	}
	if (asroot) {
		if (!(sb.st_mode & S_ISUID))
			warnx("%s(root, %s): !SUID", test_case, injail ?
			    "jail" : "!jail");
	} else {
		if (sb.st_mode & S_ISUID)
			warnx("%s(!root, %s): SUID", test_case, injail ?
			    "jail" : "!jail");
	}
}

int
priv_vfs_clearsugid_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_clearsugid_setup: fpath", fpath, UID_OWNER,
	    GID_OTHER, 0600 | S_ISUID);
	fpath_initialized = 1;
	return (0);
}

void
priv_vfs_clearsugid_chgrp(int asroot, int injail, struct test *test)
{

	if (chown(fpath, -1, asroot ? GID_WHEEL : GID_OWNER) < 0)
		err(-1, "priv_vfs_clearsugid_chgrp(%s, %s): chrgrp",
		    asroot ? "root" : "!root", injail ? "jail" : "!jail");
	confirm_sugid("priv_vfs_clearsugid_chgrp", asroot, injail);
}

#define	EA_NAMESPACE	EXTATTR_NAMESPACE_USER
#define	EA_NAME		"clearsugid"
#define	EA_DATA		"test"
#define	EA_SIZE		(strlen(EA_DATA))

void
priv_vfs_clearsugid_extattr(int asroot, int injail, struct test *test)
{

	if (extattr_set_file(fpath, EA_NAMESPACE, EA_NAME, EA_DATA, EA_SIZE)
	    < 0)
		err(-1,
		    "priv_vfs_clearsugid_extattr(%s, %s): extattr_set_file",
		    asroot ? "root" : "!root", injail ? "jail" : "!jail");
	confirm_sugid("priv_vfs_clearsugid_extattr", asroot, injail);
}

void
priv_vfs_clearsugid_write(int asroot, int injail, struct test *test)
{
	int fd;

	fd = open(fpath, O_RDWR);
	if (fd < 0)
		err(-1, "priv_vfs_clearsugid_write(%s, %s): open",
		    asroot ? "root" : "!root", injail ? "jail" : "!jail");
	if (write(fd, EA_DATA, EA_SIZE) < 0)
		err(-1, "priv_vfs_clearsugid_write(%s, %s): write",
		    asroot ? "root" : "!root", injail ? "jail" : "!jail");
	(void)close(fd);
	confirm_sugid("priv_vfs_clearsugid_write", asroot, injail);
}

void
priv_vfs_clearsugid_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}
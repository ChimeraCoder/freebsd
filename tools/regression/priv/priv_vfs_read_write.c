
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
 * This is a joint test of both the read and write privileges with respect to
 * discretionary file system access control (permissions).  Only permissions,
 * not ACL semantics, and only privilege-related checks are performed.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

static char fpath_none[1024];
static char fpath_read[1024];
static char fpath_write[1024];
static char fpath_readwrite[1024];

static int fpath_none_initialized;
static int fpath_read_initialized;
static int fpath_write_initialized;
static int fpath_readwrite_initialized;

static void
try_io(const char *label, const char *fpathp, int asroot, int injail, 
    int flags, int expected_error, int expected_errno)
{
	int fd;

	fd = open(fpathp, flags);
	if (fd < 0) {
		if (expected_error != -1)
			warnx("%s(%s, %s): expected (%d, %d) got (-1, %d)",
			    label, asroot ? "root" : "!root", injail ? "jail"
			    : "!jail", expected_error, expected_errno, errno);
	} else {
		if (expected_error == -1)
			warnx("%s(%s, %s): expected (%d, %d) got 0", label,
			    asroot ? "root" : "!root", injail ? "jail" :
			    "!jail", expected_error, expected_errno);
		(void)close(fd);
	}
}

int
priv_vfs_readwrite_fowner_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_readwrite_fowner_setup: fpath_none", fpath_none,
	    asroot ? UID_ROOT : UID_OWNER, GID_OTHER, 0000);	
	fpath_none_initialized = 1;
	setup_file("priv_vfs_readwrite_fowner_setup: fpath_read", fpath_read,
	    asroot ? UID_ROOT : UID_OWNER, GID_OTHER, 0400);
	fpath_read_initialized = 1;
	setup_file("priv_vfs_readwrite_fowner_setup: fpath_write",
	    fpath_write, asroot ? UID_ROOT : UID_OWNER, GID_OTHER, 0200);
	fpath_write_initialized = 1;
	setup_file("priv_vfs_readwrite_fowner_setup: fpath_readwrite",
	    fpath_readwrite, asroot ? UID_ROOT : UID_OWNER, GID_OTHER, 0600);
	fpath_readwrite_initialized = 1;
	return (0);
}

int
priv_vfs_readwrite_fgroup_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_readwrite_fgroup_setup: fpath_none", fpath_none,
	    UID_OTHER, asroot ? GID_WHEEL : GID_OWNER, 0000);
	fpath_none_initialized = 1;
	setup_file("priv_vfs_readwrite_fgroup_setup: fpath_read", fpath_read,
	    UID_OTHER, asroot ? GID_WHEEL : GID_OWNER, 0040);
	fpath_read_initialized = 1;
	setup_file("priv_vfs_readwrite_fgroup_setup: fpath_write",
	    fpath_write, UID_OTHER, asroot ? GID_WHEEL : GID_OWNER, 0020);
	fpath_write_initialized = 1;
	setup_file("priv_vfs_readwrite_fgroup_setup: fpath_readwrite",
	    fpath_readwrite, UID_OTHER, asroot ? GID_WHEEL : GID_OWNER,
	    0060);
	fpath_readwrite_initialized = 1;
	return (0);
}

int
priv_vfs_readwrite_fother_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_readwrite_fother_setup: fpath_none", fpath_none,
	    UID_OTHER, GID_OTHER, 0000);
	fpath_none_initialized = 1;
	setup_file("priv_vfs_readwrite_fother_setup: fpath_read", fpath_read,
	    UID_OTHER, GID_OTHER, 0004);
	fpath_read_initialized = 1;
	setup_file("priv_vfs_readwrite_fother_setup: fpath_write",
	    fpath_write, UID_OTHER, GID_OTHER, 0002);
	fpath_write_initialized = 1;
	setup_file("priv_vfs_readwrite_fother_setup: fpath_readwrite",
	    fpath_readwrite, UID_OTHER, GID_OTHER, 0006);
	fpath_readwrite_initialized = 1;
	return (0);
}

void
priv_vfs_readwrite_fowner(int asroot, int injail, struct test *test)
{

	try_io("priv_vfs_readwrite_fowner(none, O_RDONLY)", fpath_none,
	    asroot, injail, O_RDONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fowner(none, O_WRONLY)", fpath_none,
	    asroot, injail, O_WRONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fowner(none, O_RDWR)", fpath_none,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fowner(read, O_RDONLY)", fpath_read,
	    asroot, injail, O_RDONLY, 0, 0);
	try_io("priv_vfs_readwrite_fowner(read, O_WRONLY)", fpath_read,
	    asroot, injail, O_WRONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fowner(read, O_RDWR)", fpath_read,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fowner(write, O_RDONLY)", fpath_write,
	    asroot, injail, O_RDONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fowner(write, O_WRONLY)", fpath_write,
	    asroot, injail, O_WRONLY, 0, 0);
	try_io("priv_vfs_readwrite_fowner(write, O_RDWR)", fpath_write,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fowner(write, O_RDONLY)", fpath_readwrite,
	    asroot, injail, O_RDONLY, 0, 0);
	try_io("priv_vfs_readwrite_fowner(write, O_WRONLY)", fpath_readwrite,
	    asroot, injail, O_WRONLY, 0, 0);
	try_io("priv_vfs_readwrite_fowner(write, O_RDWR)", fpath_readwrite,
	    asroot, injail, O_RDWR, 0, 0);
}

void
priv_vfs_readwrite_fgroup(int asroot, int injail, struct test *test)
{

	try_io("priv_vfs_readwrite_fgroup(none, O_RDONLY)", fpath_none,
	    asroot, injail, O_RDONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fgroup(none, O_WRONLY)", fpath_none,
	    asroot, injail, O_WRONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fgroup(none, O_RDWR)", fpath_none,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fgroup(read, O_RDONLY)", fpath_read,
	    asroot, injail, O_RDONLY, 0, 0);
	try_io("priv_vfs_readwrite_fgroup(read, O_WRONLY)", fpath_read,
	    asroot, injail, O_WRONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fgroup(read, O_RDWR)", fpath_read,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fgroup(write, O_RDONLY)", fpath_write,
	    asroot, injail, O_RDONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fgroup(write, O_WRONLY)", fpath_write,
	    asroot, injail, O_WRONLY, 0, 0);
	try_io("priv_vfs_readwrite_fgroup(write, O_RDWR)", fpath_write,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fgroup(write, O_RDONLY)", fpath_readwrite,
	    asroot, injail, O_RDONLY, 0, 0);
	try_io("priv_vfs_readwrite_fgroup(write, O_WRONLY)", fpath_readwrite,
	    asroot, injail, O_WRONLY, 0, 0);
	try_io("priv_vfs_readwrite_fgroup(write, O_RDWR)", fpath_readwrite,
	    asroot, injail, O_RDWR, 0, 0);
}

void
priv_vfs_readwrite_fother(int asroot, int injail, struct test *test)
{

	try_io("priv_vfs_readwrite_fother(none, O_RDONLY)", fpath_none,
	    asroot, injail, O_RDONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fother(none, O_WRONLY)", fpath_none,
	    asroot, injail, O_WRONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fother(none, O_RDWR)", fpath_none,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fother(read, O_RDONLY)", fpath_read,
	    asroot, injail, O_RDONLY, 0, 0);
	try_io("priv_vfs_readwrite_fother(read, O_WRONLY)", fpath_read,
	    asroot, injail, O_WRONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fother(read, O_RDWR)", fpath_read,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fother(write, O_RDONLY)", fpath_write,
	    asroot, injail, O_RDONLY, asroot ? 0 : -1, EACCES);
	try_io("priv_vfs_readwrite_fother(write, O_WRONLY)", fpath_write,
	    asroot, injail, O_WRONLY, 0, 0);
	try_io("priv_vfs_readwrite_fother(write, O_RDWR)", fpath_write,
	    asroot, injail, O_RDWR, asroot ? 0 : -1, EACCES);

	try_io("priv_vfs_readwrite_fother(write, O_RDONLY)", fpath_readwrite,
	    asroot, injail, O_RDONLY, 0, 0);
	try_io("priv_vfs_readwrite_fother(write, O_WRONLY)", fpath_readwrite,
	    asroot, injail, O_WRONLY, 0, 0);
	try_io("priv_vfs_readwrite_fother(write, O_RDWR)", fpath_readwrite,
	    asroot, injail, O_RDWR, 0, 0);
}

void
priv_vfs_readwrite_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_none_initialized) {
		(void)unlink(fpath_none);
		fpath_none_initialized = 0;
	}
	if (fpath_read_initialized) {
		(void)unlink(fpath_read);
		fpath_read_initialized = 0;
	}
	if (fpath_write_initialized) {
		(void)unlink(fpath_write);
		fpath_write_initialized = 0;
	}
	if (fpath_readwrite_initialized) {
		(void)unlink(fpath_readwrite);
		fpath_readwrite_initialized = 0;
	}
}
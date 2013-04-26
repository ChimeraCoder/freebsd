
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
 * Confirm that calls to fhopen() require non-jailed priilege.  We create a
 * temporary file and grab the file handle using getfh() before starting.
 */

#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;
static fhandle_t fh;

int
priv_vfs_fhopen_setup(int asroot, int injail, struct test *test)
{

	setup_file("private_vfs_fhopen_setup: fpath", fpath, UID_ROOT,
	    GID_WHEEL, 0644);
	fpath_initialized = 1;
	if (getfh(fpath, &fh) < 0) {
		warn("priv_vfs_fhopen_setup: getfh(%s)", fpath);
		return (-1);
	}
	return (0);
}

void
priv_vfs_fhopen(int asroot, int injail, struct test *test)
{
	int errno_saved, error, fd;

	fd = fhopen(&fh, O_RDONLY);
	if (fd >= 0) {
		error = 0;
		errno_saved = errno;
		close(fd);
		errno = errno_saved;
	} else
		error = -1;
	if (asroot && injail)
		expect("priv_vfs_fhopen(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_vfs_fhopen(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_fhopen(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_fhopen(!asroot, !injail)", error, -1, EPERM);
}

void
priv_vfs_fhopen_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}
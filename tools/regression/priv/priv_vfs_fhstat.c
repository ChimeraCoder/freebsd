
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
 * Confirm that calls to fhstat() require non-jailed privilege.  We create a
 * temporary file and grab the file handle using getfh() before starting.
 */

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];
static int fpath_initialized;
static fhandle_t fh;

int
priv_vfs_fhstat_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_fhstat_setup: fpath", fpath, UID_ROOT,
	    GID_WHEEL, 0644);
	fpath_initialized = 1;
	if (getfh(fpath, &fh) < 0) {
		warn("priv_vfs_fhstat_setup: getfh(%s)", fpath);
		return (-1);
	}
	return (0);
}

void
priv_vfs_fhstat(int asroot, int injail, struct test *test)
{
	struct stat sb;
	int error;

	error = fhstat(&fh, &sb);
	if (asroot && injail)
		expect("priv_vfs_fhstat(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_vfs_fhstat(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_fhstat(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_fhstat(!asroot, !injail)", error, -1, EPERM);
}

void
priv_vfs_fhstat_cleanup(int asroot, int injail, struct test *test)
{

	if (fpath_initialized) {
		(void)unlink(fpath);
		fpath_initialized = 0;
	}
}
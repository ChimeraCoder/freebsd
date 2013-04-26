
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
 * Check that getfh() requires non-jailed privilege.
 */

#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

static char fpath[1024];

int
priv_vfs_getfh_setup(int asroot, int injail, struct test *test)
{

	setup_file("priv_vfs_getfh_setup: fpath", fpath, UID_ROOT, GID_WHEEL,
	    0644);
	return (0);
}

void
priv_vfs_getfh(int asroot, int injail, struct test *test)
{
	fhandle_t fh;
	int error;

	error = getfh(fpath, &fh);
	if (asroot && injail)
		expect("priv_vfs_getfh(asroot, injail)", error, -1, EPERM);
	if (asroot && !injail)
		expect("priv_vfs_getfh(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_getfh(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_getfh(!asroot, !injail)", error, -1, EPERM);
}

void
priv_vfs_getfh_cleanup(int asroot, int injail, struct test *test)
{

	(void)unlink(fpath);
}

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
 * Test that chroot() requires privilege--do a no-op chroot() to "/".
 *
 * XXXRW: Would also be good to check fchroot() permission, but that is not
 * exposed via the BSD API.
 */

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

int
priv_vfs_chroot_setup(int asroot, int injail, struct test *test)
{

	return (0);
}

void
priv_vfs_chroot(int asroot, int injail, struct test *test)
{
	int error;

	error = chroot("/");
	if (asroot && injail)
		expect("priv_vfs_chroot(asroot, injail)", error, 0, 0);
	if (asroot && !injail)
		expect("priv_vfs_chroot(asroot, !injail)", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vfs_chroot(!asroot, injail)", error, -1, EPERM);
	if (!asroot && !injail)
		expect("priv_vfs_chroot(!asroot, !injail)", error, -1, EPERM);
}

void
priv_vfs_chroot_cleanup(int asroot, int injail, struct test *test)
{

}
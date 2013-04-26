
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
 * Test that madvise(..., MADV_PROTECT) requires privilege.
 */

#include <sys/types.h>
#include <sys/mman.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>

#include "main.h"

int
priv_vm_madv_protect_setup(int asroot, int injail, struct test *test)
{

	return (0);
}

void
priv_vm_madv_protect(int asroot, int injail, struct test *test)
{
	int error;

	error = madvise(NULL, 0, MADV_PROTECT);
	if (asroot && injail)
		expect("priv_vm_madv_protect(asroot, injail)", error, -1,
		    EPERM);
	if (asroot && !injail)
		expect("priv_vm_madv_protect(asroot, !injail", error, 0, 0);
	if (!asroot && injail)
		expect("priv_vm_madv_protect(!asroot, injail", error, -1,
		    EPERM);
	if (!asroot && !injail)
		expect("priv_vm_madv_protect(!asroot, !injail", error, -1,
		    EPERM);
}

void
priv_vm_madv_protect_cleanup(int asroot, int injail, struct test *test)
{

}
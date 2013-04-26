
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/freebsd_rwlock.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>

const int zfs_vm_pagerret_bad = VM_PAGER_BAD;
const int zfs_vm_pagerret_error = VM_PAGER_ERROR;
const int zfs_vm_pagerret_ok = VM_PAGER_OK;

void
zfs_vmobject_assert_wlocked(vm_object_t object)
{

	/*
	 * This is not ideal because FILE/LINE used by assertions will not
	 * be too helpful, but it must be an hard function for
	 * compatibility reasons.
	 */
	VM_OBJECT_ASSERT_WLOCKED(object);
}

void
zfs_vmobject_wlock(vm_object_t object)
{

	VM_OBJECT_WLOCK(object);
}

void
zfs_vmobject_wunlock(vm_object_t object)
{

	VM_OBJECT_WUNLOCK(object);
}
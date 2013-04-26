
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
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <sys/sysctl.h>
#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_kern.h>
#include <vm/vm_pageout.h>
#include <vm/vm_extern.h>
#include <vm/uma.h>
#include <vm/uma.h>
#include <vm/uma_int.h>
#include <machine/md_var.h>
#include <machine/vmparam.h>

static int hw_uma_mdpages;
SYSCTL_INT(_hw, OID_AUTO, uma_mdpages, CTLFLAG_RD, &hw_uma_mdpages, 0,
	   "UMA MD pages in use");

void *
uma_small_alloc(uma_zone_t zone, int bytes, u_int8_t *flags, int wait)
{
	void *va;
	vm_page_t m;
	int pflags;
	
	*flags = UMA_SLAB_PRIV;
	pflags = malloc2vm_flags(wait) | VM_ALLOC_WIRED;

	for (;;) {
		m = vm_page_alloc(NULL, 0, pflags | VM_ALLOC_NOOBJ);
		if (m == NULL) {
			if (wait & M_NOWAIT)
				return (NULL);
			VM_WAIT;
		} else
			break;
	}

	va = (void *) VM_PAGE_TO_PHYS(m);

	if (!hw_direct_map)
		pmap_kenter((vm_offset_t)va, VM_PAGE_TO_PHYS(m));

	if ((wait & M_ZERO) && (m->flags & PG_ZERO) == 0)
		bzero(va, PAGE_SIZE);
	atomic_add_int(&hw_uma_mdpages, 1);

	return (va);
}

void
uma_small_free(void *mem, int size, u_int8_t flags)
{
	vm_page_t m;

	if (!hw_direct_map)
		pmap_remove(kernel_pmap,(vm_offset_t)mem,
		    (vm_offset_t)mem + PAGE_SIZE);

	m = PHYS_TO_VM_PAGE((vm_offset_t)mem);
	m->wire_count--;
	vm_page_free(m);
	atomic_subtract_int(&cnt.v_wire_count, 1);
	atomic_subtract_int(&hw_uma_mdpages, 1);
}
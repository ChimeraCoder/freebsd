
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
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_pageout.h>
#include <vm/uma.h>
#include <vm/uma_int.h>
#include <machine/md_var.h>
#include <machine/vmparam.h>

void *
uma_small_alloc(uma_zone_t zone, int bytes, u_int8_t *flags, int wait)
{
	vm_page_t m;
	vm_paddr_t pa;
	void *va;
	int pflags;

	*flags = UMA_SLAB_PRIV;
	pflags = malloc2vm_flags(wait) | VM_ALLOC_NOOBJ | VM_ALLOC_WIRED;
	for (;;) {
		m = vm_page_alloc(NULL, 0, pflags);
		if (m == NULL) {
			if (wait & M_NOWAIT)
				return (NULL);
			else
				VM_WAIT;
		} else
			break;
	}
	pa = m->phys_addr;
	if ((wait & M_NODUMP) == 0)
		dump_add_page(pa);
	va = (void *)PHYS_TO_DMAP(pa);
	if ((wait & M_ZERO) && (m->flags & PG_ZERO) == 0)
		pagezero(va);
	return (va);
}

void
uma_small_free(void *mem, int size, u_int8_t flags)
{
	vm_page_t m;
	vm_paddr_t pa;

	pa = DMAP_TO_PHYS((vm_offset_t)mem);
	dump_drop_page(pa);
	m = PHYS_TO_VM_PAGE(pa);
	m->wire_count--;
	vm_page_free(m);
	atomic_subtract_int(&cnt.v_wire_count, 1);
}
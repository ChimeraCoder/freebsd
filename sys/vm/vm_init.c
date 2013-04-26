
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
 *	Initialize the Virtual Memory subsystem.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/proc.h>
#include <sys/rwlock.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/selinfo.h>
#include <sys/pipe.h>
#include <sys/bio.h>
#include <sys/buf.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>
#include <vm/vm_pager.h>
#include <vm/vm_extern.h>

long physmem;

static int exec_map_entries = 16;
TUNABLE_INT("vm.exec_map_entries", &exec_map_entries);
SYSCTL_INT(_vm, OID_AUTO, exec_map_entries, CTLFLAG_RD, &exec_map_entries, 0,
    "Maximum number of simultaneous execs");

/*
 * System initialization
 */
static void vm_mem_init(void *);
SYSINIT(vm_mem, SI_SUB_VM, SI_ORDER_FIRST, vm_mem_init, NULL);

/*
 *	vm_init initializes the virtual memory system.
 *	This is done only by the first cpu up.
 *
 *	The start and end address of physical memory is passed in.
 */
/* ARGSUSED*/
static void
vm_mem_init(dummy)
	void *dummy;
{
	/*
	 * Initializes resident memory structures. From here on, all physical
	 * memory is accounted for, and we use only virtual addresses.
	 */
	vm_set_page_size();
	virtual_avail = vm_page_startup(virtual_avail);
	
	/*
	 * Initialize other VM packages
	 */
	vm_object_init();
	vm_map_startup();
	kmem_init(virtual_avail, virtual_end);
	pmap_init();
	vm_pager_init();
}

void
vm_ksubmap_init(struct kva_md_info *kmi)
{
	vm_offset_t firstaddr;
	caddr_t v;
	vm_size_t size = 0;
	long physmem_est;
	vm_offset_t minaddr;
	vm_offset_t maxaddr;
	vm_map_t clean_map;

	/*
	 * Allocate space for system data structures.
	 * The first available kernel virtual address is in "v".
	 * As pages of kernel virtual memory are allocated, "v" is incremented.
	 * As pages of memory are allocated and cleared,
	 * "firstaddr" is incremented.
	 * An index into the kernel page table corresponding to the
	 * virtual memory address maintained in "v" is kept in "mapaddr".
	 */

	/*
	 * Make two passes.  The first pass calculates how much memory is
	 * needed and allocates it.  The second pass assigns virtual
	 * addresses to the various data structures.
	 */
	firstaddr = 0;
again:
	v = (caddr_t)firstaddr;

	/*
	 * Discount the physical memory larger than the size of kernel_map
	 * to avoid eating up all of KVA space.
	 */
	physmem_est = lmin(physmem, btoc(kernel_map->max_offset -
	    kernel_map->min_offset));

	v = kern_vfs_bio_buffer_alloc(v, physmem_est);

	/*
	 * End of first pass, size has been calculated so allocate memory
	 */
	if (firstaddr == 0) {
		size = (vm_size_t)v;
		firstaddr = kmem_alloc(kernel_map, round_page(size));
		if (firstaddr == 0)
			panic("startup: no room for tables");
		goto again;
	}

	/*
	 * End of second pass, addresses have been assigned
	 */
	if ((vm_size_t)((char *)v - firstaddr) != size)
		panic("startup: table size inconsistency");

	clean_map = kmem_suballoc(kernel_map, &kmi->clean_sva, &kmi->clean_eva,
	    (long)nbuf * BKVASIZE + (long)nswbuf * MAXPHYS +
	    (long)bio_transient_maxcnt * MAXPHYS, TRUE);
	buffer_map = kmem_suballoc(clean_map, &kmi->buffer_sva,
	    &kmi->buffer_eva, (long)nbuf * BKVASIZE, FALSE);
	buffer_map->system_map = 1;
	if (bio_transient_maxcnt != 0) {
		bio_transient_map = kmem_suballoc(clean_map,
		    &kmi->bio_transient_sva, &kmi->bio_transient_eva,
		    (long)bio_transient_maxcnt * MAXPHYS, FALSE);
		bio_transient_map->system_map = 1;
	}
	pager_map = kmem_suballoc(clean_map, &kmi->pager_sva, &kmi->pager_eva,
	    (long)nswbuf * MAXPHYS, FALSE);
	pager_map->system_map = 1;
	exec_map = kmem_suballoc(kernel_map, &minaddr, &maxaddr,
	    exec_map_entries * round_page(PATH_MAX + ARG_MAX), FALSE);
	pipe_map = kmem_suballoc(kernel_map, &minaddr, &maxaddr, maxpipekva,
	    FALSE);

	/*
	 * XXX: Mbuf system machine-specific initializations should
	 *      go here, if anywhere.
	 */
}
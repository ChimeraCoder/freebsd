
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
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/proc.h>
#include <sys/mutex.h>
#include <sys/mman.h>
#include <sys/rwlock.h>
#include <sys/sysctl.h>

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>

/* list of phys pager objects */
static struct pagerlst phys_pager_object_list;
/* protect access to phys_pager_object_list */
static struct mtx phys_pager_mtx;

static void
phys_pager_init(void)
{

	TAILQ_INIT(&phys_pager_object_list);
	mtx_init(&phys_pager_mtx, "phys_pager list", NULL, MTX_DEF);
}

/*
 * MPSAFE
 */
static vm_object_t
phys_pager_alloc(void *handle, vm_ooffset_t size, vm_prot_t prot,
    vm_ooffset_t foff, struct ucred *cred)
{
	vm_object_t object, object1;
	vm_pindex_t pindex;

	/*
	 * Offset should be page aligned.
	 */
	if (foff & PAGE_MASK)
		return (NULL);

	pindex = OFF_TO_IDX(foff + PAGE_MASK + size);

	if (handle != NULL) {
		mtx_lock(&phys_pager_mtx);
		/*
		 * Look up pager, creating as necessary.
		 */
		object1 = NULL;
		object = vm_pager_object_lookup(&phys_pager_object_list, handle);
		if (object == NULL) {
			/*
			 * Allocate object and associate it with the pager.
			 */
			mtx_unlock(&phys_pager_mtx);
			object1 = vm_object_allocate(OBJT_PHYS, pindex);
			mtx_lock(&phys_pager_mtx);
			object = vm_pager_object_lookup(&phys_pager_object_list,
			    handle);
			if (object != NULL) {
				/*
				 * We raced with other thread while
				 * allocating object.
				 */
				if (pindex > object->size)
					object->size = pindex;
			} else {
				object = object1;
				object1 = NULL;
				object->handle = handle;
				TAILQ_INSERT_TAIL(&phys_pager_object_list, object,
				    pager_object_list);
			}
		} else {
			if (pindex > object->size)
				object->size = pindex;
		}
		mtx_unlock(&phys_pager_mtx);
		vm_object_deallocate(object1);
	} else {
		object = vm_object_allocate(OBJT_PHYS, pindex);
	}

	return (object);
}

/*
 * MPSAFE
 */
static void
phys_pager_dealloc(vm_object_t object)
{

	if (object->handle != NULL) {
		VM_OBJECT_WUNLOCK(object);
		mtx_lock(&phys_pager_mtx);
		TAILQ_REMOVE(&phys_pager_object_list, object, pager_object_list);
		mtx_unlock(&phys_pager_mtx);
		VM_OBJECT_WLOCK(object);
	}
}

/*
 * Fill as many pages as vm_fault has allocated for us.
 */
static int
phys_pager_getpages(vm_object_t object, vm_page_t *m, int count, int reqpage)
{
	int i;

	VM_OBJECT_ASSERT_WLOCKED(object);
	for (i = 0; i < count; i++) {
		if (m[i]->valid == 0) {
			if ((m[i]->flags & PG_ZERO) == 0)
				pmap_zero_page(m[i]);
			m[i]->valid = VM_PAGE_BITS_ALL;
		}
		KASSERT(m[i]->valid == VM_PAGE_BITS_ALL,
		    ("phys_pager_getpages: partially valid page %p", m[i]));
		KASSERT(m[i]->dirty == 0,
		    ("phys_pager_getpages: dirty page %p", m[i]));
		/* The requested page must remain busy, the others not. */
		if (i == reqpage)
			vm_page_flash(m[i]);
		else
			vm_page_wakeup(m[i]);
	}
	return (VM_PAGER_OK);
}

static void
phys_pager_putpages(vm_object_t object, vm_page_t *m, int count, boolean_t sync,
		    int *rtvals)
{

	panic("phys_pager_putpage called");
}

/*
 * Implement a pretty aggressive clustered getpages strategy.  Hint that
 * everything in an entire 4MB window should be prefaulted at once.
 *
 * XXX 4MB (1024 slots per page table page) is convenient for x86,
 * but may not be for other arches.
 */
#ifndef PHYSCLUSTER
#define PHYSCLUSTER 1024
#endif
static boolean_t
phys_pager_haspage(vm_object_t object, vm_pindex_t pindex, int *before,
		   int *after)
{
	vm_pindex_t base, end;

	base = pindex & (~(PHYSCLUSTER - 1));
	end = base + (PHYSCLUSTER - 1);
	if (before != NULL)
		*before = pindex - base;
	if (after != NULL)
		*after = end - pindex;
	return (TRUE);
}

struct pagerops physpagerops = {
	.pgo_init =	phys_pager_init,
	.pgo_alloc =	phys_pager_alloc,
	.pgo_dealloc = 	phys_pager_dealloc,
	.pgo_getpages =	phys_pager_getpages,
	.pgo_putpages =	phys_pager_putpages,
	.pgo_haspage =	phys_pager_haspage,
};
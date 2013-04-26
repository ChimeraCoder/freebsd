
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

/*
 * This pager manages OBJT_SG objects.  These objects are backed by
 * a scatter/gather list of physical address ranges.
 */

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/rwlock.h>
#include <sys/sglist.h>
#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>
#include <vm/vm_phys.h>
#include <vm/uma.h>

static vm_object_t sg_pager_alloc(void *, vm_ooffset_t, vm_prot_t,
    vm_ooffset_t, struct ucred *);
static void sg_pager_dealloc(vm_object_t);
static int sg_pager_getpages(vm_object_t, vm_page_t *, int, int);
static void sg_pager_putpages(vm_object_t, vm_page_t *, int, 
		boolean_t, int *);
static boolean_t sg_pager_haspage(vm_object_t, vm_pindex_t, int *,
		int *);

struct pagerops sgpagerops = {
	.pgo_alloc =	sg_pager_alloc,
	.pgo_dealloc =	sg_pager_dealloc,
	.pgo_getpages =	sg_pager_getpages,
	.pgo_putpages =	sg_pager_putpages,
	.pgo_haspage =	sg_pager_haspage,
};

static vm_object_t
sg_pager_alloc(void *handle, vm_ooffset_t size, vm_prot_t prot,
    vm_ooffset_t foff, struct ucred *cred)
{
	struct sglist *sg;
	vm_object_t object;
	vm_pindex_t npages, pindex;
	int i;

	/*
	 * Offset should be page aligned.
	 */
	if (foff & PAGE_MASK)
		return (NULL);

	/*
	 * The scatter/gather list must only include page-aligned
	 * ranges.
	 */
	npages = 0;
	sg = handle;
	for (i = 0; i < sg->sg_nseg; i++) {
		if ((sg->sg_segs[i].ss_paddr % PAGE_SIZE) != 0 ||
		    (sg->sg_segs[i].ss_len % PAGE_SIZE) != 0)
			return (NULL);
		npages += sg->sg_segs[i].ss_len / PAGE_SIZE;
	}

	/*
	 * The scatter/gather list has a fixed size.  Refuse requests
	 * to map beyond that.
	 */
	size = round_page(size);
	pindex = OFF_TO_IDX(foff + size);
	if (pindex > npages)
		return (NULL);

	/*
	 * Allocate a new object and associate it with the
	 * scatter/gather list.  It is ok for our purposes to have
	 * multiple VM objects associated with the same scatter/gather
	 * list because scatter/gather lists are static.  This is also
	 * simpler than ensuring a unique object per scatter/gather
	 * list.
	 */
	object = vm_object_allocate(OBJT_SG, npages);
	object->handle = sglist_hold(sg);
	TAILQ_INIT(&object->un_pager.sgp.sgp_pglist);
	return (object);
}

static void
sg_pager_dealloc(vm_object_t object)
{
	struct sglist *sg;
	vm_page_t m;

	/*
	 * Free up our fake pages.
	 */
	while ((m = TAILQ_FIRST(&object->un_pager.sgp.sgp_pglist)) != 0) {
		TAILQ_REMOVE(&object->un_pager.sgp.sgp_pglist, m, pageq);
		vm_page_putfake(m);
	}
	
	sg = object->handle;
	sglist_free(sg);
}

static int
sg_pager_getpages(vm_object_t object, vm_page_t *m, int count, int reqpage)
{
	struct sglist *sg;
	vm_page_t m_paddr, page;
	vm_pindex_t offset;
	vm_paddr_t paddr;
	vm_memattr_t memattr;
	size_t space;
	int i;

	VM_OBJECT_ASSERT_WLOCKED(object);
	sg = object->handle;
	memattr = object->memattr;
	VM_OBJECT_WUNLOCK(object);
	offset = m[reqpage]->pindex;

	/*
	 * Lookup the physical address of the requested page.  An initial
	 * value of '1' instead of '0' is used so we can assert that the
	 * page is found since '0' can be a valid page-aligned physical
	 * address.
	 */
	space = 0;
	paddr = 1;
	for (i = 0; i < sg->sg_nseg; i++) {
		if (space + sg->sg_segs[i].ss_len <= (offset * PAGE_SIZE)) {
			space += sg->sg_segs[i].ss_len;
			continue;
		}
		paddr = sg->sg_segs[i].ss_paddr + offset * PAGE_SIZE - space;
		break;
	}
	KASSERT(paddr != 1, ("invalid SG page index"));

	/* If "paddr" is a real page, perform a sanity check on "memattr". */
	if ((m_paddr = vm_phys_paddr_to_vm_page(paddr)) != NULL &&
	    pmap_page_get_memattr(m_paddr) != memattr) {
		memattr = pmap_page_get_memattr(m_paddr);
		printf(
	    "WARNING: A device driver has set \"memattr\" inconsistently.\n");
	}

	/* Return a fake page for the requested page. */
	KASSERT(!(m[reqpage]->flags & PG_FICTITIOUS),
	    ("backing page for SG is fake"));

	/* Construct a new fake page. */
	page = vm_page_getfake(paddr, memattr);
	VM_OBJECT_WLOCK(object);
	TAILQ_INSERT_TAIL(&object->un_pager.sgp.sgp_pglist, page, pageq);

	/* Free the original pages and insert this fake page into the object. */
	for (i = 0; i < count; i++) {
		vm_page_lock(m[i]);
		vm_page_free(m[i]);
		vm_page_unlock(m[i]);
	}
	vm_page_insert(page, object, offset);
	m[reqpage] = page;
	page->valid = VM_PAGE_BITS_ALL;

	return (VM_PAGER_OK);
}

static void
sg_pager_putpages(vm_object_t object, vm_page_t *m, int count,
    boolean_t sync, int *rtvals)
{

	panic("sg_pager_putpage called");
}

static boolean_t
sg_pager_haspage(vm_object_t object, vm_pindex_t pindex, int *before,
    int *after)
{

	if (before != NULL)
		*before = 0;
	if (after != NULL)
		*after = 0;
	return (TRUE);
}
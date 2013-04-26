
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
 *	Paging space routine stubs.  Emulates a matchmaker-like interface
 *	for builtin pagers.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/ucred.h>
#include <sys/malloc.h>
#include <sys/rwlock.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>
#include <vm/vm_extern.h>

int cluster_pbuf_freecnt = -1;	/* unlimited to begin with */

static int dead_pager_getpages(vm_object_t, vm_page_t *, int, int);
static vm_object_t dead_pager_alloc(void *, vm_ooffset_t, vm_prot_t,
    vm_ooffset_t, struct ucred *);
static void dead_pager_putpages(vm_object_t, vm_page_t *, int, int, int *);
static boolean_t dead_pager_haspage(vm_object_t, vm_pindex_t, int *, int *);
static void dead_pager_dealloc(vm_object_t);

static int
dead_pager_getpages(obj, ma, count, req)
	vm_object_t obj;
	vm_page_t *ma;
	int count;
	int req;
{
	return VM_PAGER_FAIL;
}

static vm_object_t
dead_pager_alloc(void *handle, vm_ooffset_t size, vm_prot_t prot,
    vm_ooffset_t off, struct ucred *cred)
{
	return NULL;
}

static void
dead_pager_putpages(object, m, count, flags, rtvals)
	vm_object_t object;
	vm_page_t *m;
	int count;
	int flags;
	int *rtvals;
{
	int i;

	for (i = 0; i < count; i++) {
		rtvals[i] = VM_PAGER_AGAIN;
	}
}

static int
dead_pager_haspage(object, pindex, prev, next)
	vm_object_t object;
	vm_pindex_t pindex;
	int *prev;
	int *next;
{
	if (prev)
		*prev = 0;
	if (next)
		*next = 0;
	return FALSE;
}

static void
dead_pager_dealloc(object)
	vm_object_t object;
{
	return;
}

static struct pagerops deadpagerops = {
	.pgo_alloc = 	dead_pager_alloc,
	.pgo_dealloc =	dead_pager_dealloc,
	.pgo_getpages =	dead_pager_getpages,
	.pgo_putpages =	dead_pager_putpages,
	.pgo_haspage =	dead_pager_haspage,
};

struct pagerops *pagertab[] = {
	&defaultpagerops,	/* OBJT_DEFAULT */
	&swappagerops,		/* OBJT_SWAP */
	&vnodepagerops,		/* OBJT_VNODE */
	&devicepagerops,	/* OBJT_DEVICE */
	&physpagerops,		/* OBJT_PHYS */
	&deadpagerops,		/* OBJT_DEAD */
	&sgpagerops,		/* OBJT_SG */
	&mgtdevicepagerops,	/* OBJT_MGTDEVICE */
};

static const int npagers = sizeof(pagertab) / sizeof(pagertab[0]);

/*
 * Kernel address space for mapping pages.
 * Used by pagers where KVAs are needed for IO.
 *
 * XXX needs to be large enough to support the number of pending async
 * cleaning requests (NPENDINGIO == 64) * the maximum swap cluster size
 * (MAXPHYS == 64k) if you want to get the most efficiency.
 */
vm_map_t pager_map;
static int bswneeded;
static vm_offset_t swapbkva;		/* swap buffers kva */
struct mtx pbuf_mtx;
static TAILQ_HEAD(swqueue, buf) bswlist;

void
vm_pager_init()
{
	struct pagerops **pgops;

	TAILQ_INIT(&bswlist);
	/*
	 * Initialize known pagers
	 */
	for (pgops = pagertab; pgops < &pagertab[npagers]; pgops++)
		if ((*pgops)->pgo_init != NULL)
			(*(*pgops)->pgo_init) ();
}

void
vm_pager_bufferinit()
{
	struct buf *bp;
	int i;

	mtx_init(&pbuf_mtx, "pbuf mutex", NULL, MTX_DEF);
	bp = swbuf;
	/*
	 * Now set up swap and physical I/O buffer headers.
	 */
	for (i = 0; i < nswbuf; i++, bp++) {
		TAILQ_INSERT_HEAD(&bswlist, bp, b_freelist);
		BUF_LOCKINIT(bp);
		LIST_INIT(&bp->b_dep);
		bp->b_rcred = bp->b_wcred = NOCRED;
		bp->b_xflags = 0;
	}

	cluster_pbuf_freecnt = nswbuf / 2;
	vnode_pbuf_freecnt = nswbuf / 2 + 1;

	swapbkva = kmem_alloc_nofault(pager_map, nswbuf * MAXPHYS);
	if (!swapbkva)
		panic("Not enough pager_map VM space for physical buffers");
}

/*
 * Allocate an instance of a pager of the given type.
 * Size, protection and offset parameters are passed in for pagers that
 * need to perform page-level validation (e.g. the device pager).
 */
vm_object_t
vm_pager_allocate(objtype_t type, void *handle, vm_ooffset_t size,
    vm_prot_t prot, vm_ooffset_t off, struct ucred *cred)
{
	vm_object_t ret;
	struct pagerops *ops;

	ops = pagertab[type];
	if (ops)
		ret = (*ops->pgo_alloc) (handle, size, prot, off, cred);
	else
		ret = NULL;
	return (ret);
}

/*
 *	The object must be locked.
 */
void
vm_pager_deallocate(object)
	vm_object_t object;
{

	VM_OBJECT_ASSERT_WLOCKED(object);
	(*pagertab[object->type]->pgo_dealloc) (object);
}

/*
 * vm_pager_get_pages() - inline, see vm/vm_pager.h
 * vm_pager_put_pages() - inline, see vm/vm_pager.h
 * vm_pager_has_page() - inline, see vm/vm_pager.h
 */

/*
 * Search the specified pager object list for an object with the
 * specified handle.  If an object with the specified handle is found,
 * increase its reference count and return it.  Otherwise, return NULL.
 *
 * The pager object list must be locked.
 */
vm_object_t
vm_pager_object_lookup(struct pagerlst *pg_list, void *handle)
{
	vm_object_t object;

	TAILQ_FOREACH(object, pg_list, pager_object_list) {
		if (object->handle == handle) {
			VM_OBJECT_WLOCK(object);
			if ((object->flags & OBJ_DEAD) == 0) {
				vm_object_reference_locked(object);
				VM_OBJECT_WUNLOCK(object);
				break;
			}
			VM_OBJECT_WUNLOCK(object);
		}
	}
	return (object);
}

/*
 * initialize a physical buffer
 */

/*
 * XXX This probably belongs in vfs_bio.c
 */
static void
initpbuf(struct buf *bp)
{
	KASSERT(bp->b_bufobj == NULL, ("initpbuf with bufobj"));
	KASSERT(bp->b_vp == NULL, ("initpbuf with vp"));
	bp->b_rcred = NOCRED;
	bp->b_wcred = NOCRED;
	bp->b_qindex = 0;	/* On no queue (QUEUE_NONE) */
	bp->b_saveaddr = (caddr_t) (MAXPHYS * (bp - swbuf)) + swapbkva;
	bp->b_data = bp->b_saveaddr;
	bp->b_kvabase = bp->b_saveaddr;
	bp->b_kvasize = MAXPHYS;
	bp->b_xflags = 0;
	bp->b_flags = 0;
	bp->b_ioflags = 0;
	bp->b_iodone = NULL;
	bp->b_error = 0;
	BUF_LOCK(bp, LK_EXCLUSIVE, NULL);
}

/*
 * allocate a physical buffer
 *
 *	There are a limited number (nswbuf) of physical buffers.  We need
 *	to make sure that no single subsystem is able to hog all of them,
 *	so each subsystem implements a counter which is typically initialized
 *	to 1/2 nswbuf.  getpbuf() decrements this counter in allocation and
 *	increments it on release, and blocks if the counter hits zero.  A
 *	subsystem may initialize the counter to -1 to disable the feature,
 *	but it must still be sure to match up all uses of getpbuf() with 
 *	relpbuf() using the same variable.
 *
 *	NOTE: pfreecnt can be NULL, but this 'feature' will be removed
 *	relatively soon when the rest of the subsystems get smart about it. XXX
 */
struct buf *
getpbuf(int *pfreecnt)
{
	struct buf *bp;

	mtx_lock(&pbuf_mtx);

	for (;;) {
		if (pfreecnt) {
			while (*pfreecnt == 0) {
				msleep(pfreecnt, &pbuf_mtx, PVM, "wswbuf0", 0);
			}
		}

		/* get a bp from the swap buffer header pool */
		if ((bp = TAILQ_FIRST(&bswlist)) != NULL)
			break;

		bswneeded = 1;
		msleep(&bswneeded, &pbuf_mtx, PVM, "wswbuf1", 0);
		/* loop in case someone else grabbed one */
	}
	TAILQ_REMOVE(&bswlist, bp, b_freelist);
	if (pfreecnt)
		--*pfreecnt;
	mtx_unlock(&pbuf_mtx);

	initpbuf(bp);
	return bp;
}

/*
 * allocate a physical buffer, if one is available.
 *
 *	Note that there is no NULL hack here - all subsystems using this
 *	call understand how to use pfreecnt.
 */
struct buf *
trypbuf(int *pfreecnt)
{
	struct buf *bp;

	mtx_lock(&pbuf_mtx);
	if (*pfreecnt == 0 || (bp = TAILQ_FIRST(&bswlist)) == NULL) {
		mtx_unlock(&pbuf_mtx);
		return NULL;
	}
	TAILQ_REMOVE(&bswlist, bp, b_freelist);

	--*pfreecnt;

	mtx_unlock(&pbuf_mtx);

	initpbuf(bp);

	return bp;
}

/*
 * release a physical buffer
 *
 *	NOTE: pfreecnt can be NULL, but this 'feature' will be removed
 *	relatively soon when the rest of the subsystems get smart about it. XXX
 */
void
relpbuf(struct buf *bp, int *pfreecnt)
{

	if (bp->b_rcred != NOCRED) {
		crfree(bp->b_rcred);
		bp->b_rcred = NOCRED;
	}
	if (bp->b_wcred != NOCRED) {
		crfree(bp->b_wcred);
		bp->b_wcred = NOCRED;
	}

	KASSERT(bp->b_vp == NULL, ("relpbuf with vp"));
	KASSERT(bp->b_bufobj == NULL, ("relpbuf with bufobj"));

	BUF_UNLOCK(bp);

	mtx_lock(&pbuf_mtx);
	TAILQ_INSERT_HEAD(&bswlist, bp, b_freelist);

	if (bswneeded) {
		bswneeded = 0;
		wakeup(&bswneeded);
	}
	if (pfreecnt) {
		if (++*pfreecnt == 1)
			wakeup(pfreecnt);
	}
	mtx_unlock(&pbuf_mtx);
}

/*
 * Associate a p-buffer with a vnode.
 *
 * Also sets B_PAGING flag to indicate that vnode is not fully associated
 * with the buffer.  i.e. the bp has not been linked into the vnode or
 * ref-counted.
 */
void
pbgetvp(struct vnode *vp, struct buf *bp)
{

	KASSERT(bp->b_vp == NULL, ("pbgetvp: not free"));
	KASSERT(bp->b_bufobj == NULL, ("pbgetvp: not free (bufobj)"));

	bp->b_vp = vp;
	bp->b_flags |= B_PAGING;
	bp->b_bufobj = &vp->v_bufobj;
}

/*
 * Associate a p-buffer with a vnode.
 *
 * Also sets B_PAGING flag to indicate that vnode is not fully associated
 * with the buffer.  i.e. the bp has not been linked into the vnode or
 * ref-counted.
 */
void
pbgetbo(struct bufobj *bo, struct buf *bp)
{

	KASSERT(bp->b_vp == NULL, ("pbgetbo: not free (vnode)"));
	KASSERT(bp->b_bufobj == NULL, ("pbgetbo: not free (bufobj)"));

	bp->b_flags |= B_PAGING;
	bp->b_bufobj = bo;
}

/*
 * Disassociate a p-buffer from a vnode.
 */
void
pbrelvp(struct buf *bp)
{

	KASSERT(bp->b_vp != NULL, ("pbrelvp: NULL"));
	KASSERT(bp->b_bufobj != NULL, ("pbrelvp: NULL bufobj"));
	KASSERT((bp->b_xflags & (BX_VNDIRTY | BX_VNCLEAN)) == 0,
	    ("pbrelvp: pager buf on vnode list."));

	bp->b_vp = NULL;
	bp->b_bufobj = NULL;
	bp->b_flags &= ~B_PAGING;
}

/*
 * Disassociate a p-buffer from a bufobj.
 */
void
pbrelbo(struct buf *bp)
{

	KASSERT(bp->b_vp == NULL, ("pbrelbo: vnode"));
	KASSERT(bp->b_bufobj != NULL, ("pbrelbo: NULL bufobj"));
	KASSERT((bp->b_xflags & (BX_VNDIRTY | BX_VNCLEAN)) == 0,
	    ("pbrelbo: pager buf on vnode list."));

	bp->b_bufobj = NULL;
	bp->b_flags &= ~B_PAGING;
}
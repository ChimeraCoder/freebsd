
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <machine/stdarg.h>

#include <linux/bitops.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/idr.h>
#include <linux/err.h>

/*
 * IDR Implementation.
 *
 * This is quick and dirty and not as re-entrant as the linux version
 * however it should be fairly fast.  It is basically a radix tree with
 * a builtin bitmap for allocation.
 */
static MALLOC_DEFINE(M_IDR, "idr", "Linux IDR compat");

static inline int
idr_max(struct idr *idr)
{
	return (1 << (idr->layers * IDR_BITS)) - 1;
}

static inline int
idr_pos(int id, int layer)
{
	return (id >> (IDR_BITS * layer)) & IDR_MASK;
}

void
idr_init(struct idr *idr)
{
	bzero(idr, sizeof(*idr));
	mtx_init(&idr->lock, "idr", NULL, MTX_DEF);
}

/* Only frees cached pages. */
void
idr_destroy(struct idr *idr)
{
	struct idr_layer *il, *iln;

	mtx_lock(&idr->lock);
	for (il = idr->free; il != NULL; il = iln) {
		iln = il->ary[0];
		free(il, M_IDR);
	}
	mtx_unlock(&idr->lock);
}

static void
idr_remove_layer(struct idr_layer *il, int layer)
{
	int i;

	if (il == NULL)
		return;
	if (layer == 0) {
		free(il, M_IDR);
		return;
	}
	for (i = 0; i < IDR_SIZE; i++)
		if (il->ary[i])
			idr_remove_layer(il->ary[i], layer - 1);
}

void
idr_remove_all(struct idr *idr)
{

	mtx_lock(&idr->lock);
	idr_remove_layer(idr->top, idr->layers - 1);
	idr->top = NULL;
	idr->layers = 0;
	mtx_unlock(&idr->lock);
}

void
idr_remove(struct idr *idr, int id)
{
	struct idr_layer *il;
	int layer;
	int idx;

	id &= MAX_ID_MASK;
	mtx_lock(&idr->lock);
	il = idr->top;
	layer = idr->layers - 1;
	if (il == NULL || id > idr_max(idr)) {
		mtx_unlock(&idr->lock);
		return;
	}
	/*
	 * Walk down the tree to this item setting bitmaps along the way
	 * as we know at least one item will be free along this path.
	 */
	while (layer && il) {
		idx = idr_pos(id, layer);
		il->bitmap |= 1 << idx;
		il = il->ary[idx];
		layer--;
	}
	idx = id & IDR_MASK;
	/*
	 * At this point we've set free space bitmaps up the whole tree.
	 * We could make this non-fatal and unwind but linux dumps a stack
	 * and a warning so I don't think it's necessary.
	 */
	if (il == NULL || (il->bitmap & (1 << idx)) != 0)
		panic("idr_remove: Item %d not allocated (%p, %p)\n",
		    id, idr, il);
	il->ary[idx] = NULL;
	il->bitmap |= 1 << idx;
	mtx_unlock(&idr->lock);
	return;
}

void *
idr_replace(struct idr *idr, void *ptr, int id)
{
	struct idr_layer *il;
	void *res;
	int layer;
	int idx;

	res = ERR_PTR(-EINVAL);
	id &= MAX_ID_MASK;
	mtx_lock(&idr->lock);
	il = idr->top;
	layer = idr->layers - 1;
	if (il == NULL || id > idr_max(idr))
		goto out;
	while (layer && il) {
		il = il->ary[idr_pos(id, layer)];
		layer--;
	}
	idx = id & IDR_MASK;
	/*
	 * Replace still returns an error if the item was not allocated.
	 */
	if (il != NULL && (il->bitmap & (1 << idx)) != 0) {
		res = il->ary[idx];
		il->ary[idx] = ptr;
	}
out:
	mtx_unlock(&idr->lock);
	return (res);
}

void *
idr_find(struct idr *idr, int id)
{
	struct idr_layer *il;
	void *res;
	int layer;

	res = NULL;
	id &= MAX_ID_MASK;
	mtx_lock(&idr->lock);
	il = idr->top;
	layer = idr->layers - 1;
	if (il == NULL || id > idr_max(idr))
		goto out;
	while (layer && il) {
		il = il->ary[idr_pos(id, layer)];
		layer--;
	}
	if (il != NULL)
		res = il->ary[id & IDR_MASK];
out:
	mtx_unlock(&idr->lock);
	return (res);
}

int
idr_pre_get(struct idr *idr, gfp_t gfp_mask)
{
	struct idr_layer *il, *iln;
	struct idr_layer *head;
	int need;

	mtx_lock(&idr->lock);
	for (;;) {
		need = idr->layers + 1;
		for (il = idr->free; il != NULL; il = il->ary[0])
			need--;
		mtx_unlock(&idr->lock);
		if (need == 0)
			break;
		for (head = NULL; need; need--) {
			iln = malloc(sizeof(*il), M_IDR, M_ZERO | gfp_mask);
			if (iln == NULL)
				break;
			bitmap_fill(&iln->bitmap, IDR_SIZE);
			if (head != NULL) {
				il->ary[0] = iln;
				il = iln;
			} else
				head = il = iln;
		}
		if (head == NULL)
			return (0);
		mtx_lock(&idr->lock);
		il->ary[0] = idr->free;
		idr->free = head;
	}
	return (1);
}

static inline struct idr_layer *
idr_get(struct idr *idr)
{
	struct idr_layer *il;

	il = idr->free;
	if (il) {
		idr->free = il->ary[0];
		il->ary[0] = NULL;
		return (il);
	}
	il = malloc(sizeof(*il), M_IDR, M_ZERO | M_NOWAIT);
	bitmap_fill(&il->bitmap, IDR_SIZE);
	return (il);
}

/*
 * Could be implemented as get_new_above(idr, ptr, 0, idp) but written
 * first for simplicity sake.
 */
int
idr_get_new(struct idr *idr, void *ptr, int *idp)
{
	struct idr_layer *stack[MAX_LEVEL];
	struct idr_layer *il;
	int error;
	int layer;
	int idx;
	int id;

	error = -EAGAIN;
	mtx_lock(&idr->lock);
	/*
	 * Expand the tree until there is free space.
	 */
	if (idr->top == NULL || idr->top->bitmap == 0) {
		if (idr->layers == MAX_LEVEL + 1) {
			error = -ENOSPC;
			goto out;
		}
		il = idr_get(idr);
		if (il == NULL)
			goto out;
		il->ary[0] = idr->top;
		if (idr->top)
			il->bitmap &= ~1;
		idr->top = il;
		idr->layers++;
	}
	il = idr->top;
	id = 0;
	/*
	 * Walk the tree following free bitmaps, record our path.
	 */
	for (layer = idr->layers - 1;; layer--) {
		stack[layer] = il;
		idx = ffsl(il->bitmap);
		if (idx == 0)
			panic("idr_get_new: Invalid leaf state (%p, %p)\n",
			    idr, il);
		idx--;
		id |= idx << (layer * IDR_BITS);
		if (layer == 0)
			break;
		if (il->ary[idx] == NULL) {
			il->ary[idx] = idr_get(idr);
			if (il->ary[idx] == NULL)
				goto out;
		}
		il = il->ary[idx];
	}
	/*
	 * Allocate the leaf to the consumer.
	 */
	il->bitmap &= ~(1 << idx);
	il->ary[idx] = ptr;
	*idp = id;
	/*
	 * Clear bitmaps potentially up to the root.
	 */
	while (il->bitmap == 0 && ++layer < idr->layers) {
		il = stack[layer];
		il->bitmap &= ~(1 << idr_pos(id, layer));
	}
	error = 0;
out:
	mtx_unlock(&idr->lock);
#ifdef INVARIANTS
	if (error == 0 && idr_find(idr, id) != ptr) {
		panic("idr_get_new: Failed for idr %p, id %d, ptr %p\n",
		    idr, id, ptr);
	}
#endif
	return (error);
}

int
idr_get_new_above(struct idr *idr, void *ptr, int starting_id, int *idp)
{
	struct idr_layer *stack[MAX_LEVEL];
	struct idr_layer *il;
	int error;
	int layer;
	int idx, sidx;
	int id;

	error = -EAGAIN;
	mtx_lock(&idr->lock);
	/*
	 * Compute the layers required to support starting_id and the mask
	 * at the top layer.
	 */
restart:
	idx = starting_id;
	layer = 0;
	while (idx & ~IDR_MASK) {
		layer++;
		idx >>= IDR_BITS;
	}
	if (layer == MAX_LEVEL + 1) {
		error = -ENOSPC;
		goto out;
	}
	/*
	 * Expand the tree until there is free space at or beyond starting_id.
	 */
	while (idr->layers <= layer ||
	    idr->top->bitmap < (1 << idr_pos(starting_id, idr->layers - 1))) {
		if (idr->layers == MAX_LEVEL + 1) {
			error = -ENOSPC;
			goto out;
		}
		il = idr_get(idr);
		if (il == NULL)
			goto out;
		il->ary[0] = idr->top;
		if (idr->top && idr->top->bitmap == 0)
			il->bitmap &= ~1;
		idr->top = il;
		idr->layers++;
	}
	il = idr->top;
	id = 0;
	/*
	 * Walk the tree following free bitmaps, record our path.
	 */
	for (layer = idr->layers - 1;; layer--) {
		stack[layer] = il;
		sidx = idr_pos(starting_id, layer);
		/* Returns index numbered from 0 or size if none exists. */
		idx = find_next_bit(&il->bitmap, IDR_SIZE, sidx);
		if (idx == IDR_SIZE && sidx == 0)
			panic("idr_get_new: Invalid leaf state (%p, %p)\n",
			    idr, il);
		/*
		 * We may have walked a path where there was a free bit but
		 * it was lower than what we wanted.  Restart the search with
		 * a larger starting id.  id contains the progress we made so
		 * far.  Search the leaf one above this level.  This may
		 * restart as many as MAX_LEVEL times but that is expected
		 * to be rare.
		 */
		if (idx == IDR_SIZE) {
			starting_id = id + (1 << (layer+1 * IDR_BITS));
			goto restart;
		}
		if (idx > sidx)
			starting_id = 0;	/* Search the whole subtree. */
		id |= idx << (layer * IDR_BITS);
		if (layer == 0)
			break;
		if (il->ary[idx] == NULL) {
			il->ary[idx] = idr_get(idr);
			if (il->ary[idx] == NULL)
				goto out;
		}
		il = il->ary[idx];
	}
	/*
	 * Allocate the leaf to the consumer.
	 */
	il->bitmap &= ~(1 << idx);
	il->ary[idx] = ptr;
	*idp = id;
	/*
	 * Clear bitmaps potentially up to the root.
	 */
	while (il->bitmap == 0 && ++layer < idr->layers) {
		il = stack[layer];
		il->bitmap &= ~(1 << idr_pos(id, layer));
	}
	error = 0;
out:
	mtx_unlock(&idr->lock);
#ifdef INVARIANTS
	if (error == 0 && idr_find(idr, id) != ptr) {
		panic("idr_get_new_above: Failed for idr %p, id %d, ptr %p\n",
		    idr, id, ptr);
	}
#endif
	return (error);
}
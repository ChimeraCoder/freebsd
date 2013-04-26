
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
 * uma_dbg.c	Debugging features for UMA users
 *
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/malloc.h>

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>
#include <vm/uma.h>
#include <vm/uma_int.h>
#include <vm/uma_dbg.h>

static const uint32_t uma_junk = 0xdeadc0de;

/*
 * Checks an item to make sure it hasn't been overwritten since it was freed,
 * prior to subsequent reallocation.
 *
 * Complies with standard ctor arg/return
 *
 */
int
trash_ctor(void *mem, int size, void *arg, int flags)
{
	int cnt;
	uint32_t *p;

	cnt = size / sizeof(uma_junk);

	for (p = mem; cnt > 0; cnt--, p++)
		if (*p != uma_junk) {
			printf("Memory modified after free %p(%d) val=%x @ %p\n",
			    mem, size, *p, p);
			return (0);
		}
	return (0);
}

/*
 * Fills an item with predictable garbage
 *
 * Complies with standard dtor arg/return
 *
 */
void
trash_dtor(void *mem, int size, void *arg)
{
	int cnt;
	uint32_t *p;

	cnt = size / sizeof(uma_junk);

	for (p = mem; cnt > 0; cnt--, p++)
		*p = uma_junk;
}

/*
 * Fills an item with predictable garbage
 *
 * Complies with standard init arg/return
 *
 */
int
trash_init(void *mem, int size, int flags)
{
	trash_dtor(mem, size, NULL);
	return (0);
}

/*
 * Checks an item to make sure it hasn't been overwritten since it was freed.
 *
 * Complies with standard fini arg/return
 *
 */
void
trash_fini(void *mem, int size)
{
	(void)trash_ctor(mem, size, NULL, 0);
}

int
mtrash_ctor(void *mem, int size, void *arg, int flags)
{
	struct malloc_type **ksp;
	uint32_t *p = mem;
	int cnt;

	size -= sizeof(struct malloc_type *);
	ksp = (struct malloc_type **)mem;
	ksp += size / sizeof(struct malloc_type *);
	cnt = size / sizeof(uma_junk);

	for (p = mem; cnt > 0; cnt--, p++)
		if (*p != uma_junk) {
			printf("Memory modified after free %p(%d) val=%x @ %p\n",
			    mem, size, *p, p);
			panic("Most recently used by %s\n", (*ksp == NULL)?
			    "none" : (*ksp)->ks_shortdesc);
		}
	return (0);
}

/*
 * Fills an item with predictable garbage
 *
 * Complies with standard dtor arg/return
 *
 */
void
mtrash_dtor(void *mem, int size, void *arg)
{
	int cnt;
	uint32_t *p;

	size -= sizeof(struct malloc_type *);
	cnt = size / sizeof(uma_junk);

	for (p = mem; cnt > 0; cnt--, p++)
		*p = uma_junk;
}

/*
 * Fills an item with predictable garbage
 *
 * Complies with standard init arg/return
 *
 */
int
mtrash_init(void *mem, int size, int flags)
{
	struct malloc_type **ksp;

	mtrash_dtor(mem, size, NULL);

	ksp = (struct malloc_type **)mem;
	ksp += (size / sizeof(struct malloc_type *)) - 1;
	*ksp = NULL;
	return (0);
}

/*
 * Checks an item to make sure it hasn't been overwritten since it was freed,
 * prior to freeing it back to available memory.
 *
 * Complies with standard fini arg/return
 *
 */
void
mtrash_fini(void *mem, int size)
{
	(void)mtrash_ctor(mem, size, NULL, 0);
}

static uma_slab_t
uma_dbg_getslab(uma_zone_t zone, void *item)
{
	uma_slab_t slab;
	uma_keg_t keg;
	uint8_t *mem;

	mem = (uint8_t *)((unsigned long)item & (~UMA_SLAB_MASK));
	if (zone->uz_flags & UMA_ZONE_VTOSLAB) {
		slab = vtoslab((vm_offset_t)mem);
	} else {
		keg = LIST_FIRST(&zone->uz_kegs)->kl_keg;
		if (keg->uk_flags & UMA_ZONE_HASH)
			slab = hash_sfind(&keg->uk_hash, mem);
		else
			slab = (uma_slab_t)(mem + keg->uk_pgoff);
	}

	return (slab);
}

/*
 * Set up the slab's freei data such that uma_dbg_free can function.
 *
 */

void
uma_dbg_alloc(uma_zone_t zone, uma_slab_t slab, void *item)
{
	uma_keg_t keg;
	uma_slabrefcnt_t slabref;
	int freei;

	if (slab == NULL) {
		slab = uma_dbg_getslab(zone, item);
		if (slab == NULL) 
			panic("uma: item %p did not belong to zone %s\n",
			    item, zone->uz_name);
	}
	keg = slab->us_keg;

	freei = ((unsigned long)item - (unsigned long)slab->us_data)
	    / keg->uk_rsize;

	if (keg->uk_flags & UMA_ZONE_REFCNT) {
		slabref = (uma_slabrefcnt_t)slab;
		slabref->us_freelist[freei].us_item = 255;
	} else {
		slab->us_freelist[freei].us_item = 255;
	}

	return;
}

/*
 * Verifies freed addresses.  Checks for alignment, valid slab membership
 * and duplicate frees.
 *
 */

void
uma_dbg_free(uma_zone_t zone, uma_slab_t slab, void *item)
{
	uma_keg_t keg;
	uma_slabrefcnt_t slabref;
	int freei;

	if (slab == NULL) {
		slab = uma_dbg_getslab(zone, item);
		if (slab == NULL) 
			panic("uma: Freed item %p did not belong to zone %s\n",
			    item, zone->uz_name);
	}
	keg = slab->us_keg;

	freei = ((unsigned long)item - (unsigned long)slab->us_data)
	    / keg->uk_rsize;

	if (freei >= keg->uk_ipers)
		panic("zone: %s(%p) slab %p freelist %d out of range 0-%d\n",
		    zone->uz_name, zone, slab, freei, keg->uk_ipers-1);

	if (((freei * keg->uk_rsize) + slab->us_data) != item) {
		printf("zone: %s(%p) slab %p freed address %p unaligned.\n",
		    zone->uz_name, zone, slab, item);
		panic("should be %p\n",
		    (freei * keg->uk_rsize) + slab->us_data);
	}

	if (keg->uk_flags & UMA_ZONE_REFCNT) {
		slabref = (uma_slabrefcnt_t)slab;
		if (slabref->us_freelist[freei].us_item != 255) {
			printf("Slab at %p, freei %d = %d.\n",
			    slab, freei, slabref->us_freelist[freei].us_item);
			panic("Duplicate free of item %p from zone %p(%s)\n",
			    item, zone, zone->uz_name);
		}

		/*
		 * When this is actually linked into the slab this will change.
		 * Until then the count of valid slabs will make sure we don't
		 * accidentally follow this and assume it's a valid index.
		 */
		slabref->us_freelist[freei].us_item = 0;
	} else {
		if (slab->us_freelist[freei].us_item != 255) {
			printf("Slab at %p, freei %d = %d.\n",
			    slab, freei, slab->us_freelist[freei].us_item);
			panic("Duplicate free of item %p from zone %p(%s)\n",
			    item, zone, zone->uz_name);
		}

		/*
		 * When this is actually linked into the slab this will change.
		 * Until then the count of valid slabs will make sure we don't
		 * accidentally follow this and assume it's a valid index.
		 */
		slab->us_freelist[freei].us_item = 0;
	}
}
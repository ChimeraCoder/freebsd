
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

#include <sys/bplist.h>
#include <sys/zfs_context.h>


void
bplist_create(bplist_t *bpl)
{
	mutex_init(&bpl->bpl_lock, NULL, MUTEX_DEFAULT, NULL);
	list_create(&bpl->bpl_list, sizeof (bplist_entry_t),
	    offsetof(bplist_entry_t, bpe_node));
}

void
bplist_destroy(bplist_t *bpl)
{
	list_destroy(&bpl->bpl_list);
	mutex_destroy(&bpl->bpl_lock);
}

void
bplist_append(bplist_t *bpl, const blkptr_t *bp)
{
	bplist_entry_t *bpe = kmem_alloc(sizeof (*bpe), KM_SLEEP);

	mutex_enter(&bpl->bpl_lock);
	bpe->bpe_blk = *bp;
	list_insert_tail(&bpl->bpl_list, bpe);
	mutex_exit(&bpl->bpl_lock);
}

/*
 * To aid debugging, we keep the most recently removed entry.  This way if
 * we are in the callback, we can easily locate the entry.
 */
static bplist_entry_t *bplist_iterate_last_removed;

void
bplist_iterate(bplist_t *bpl, bplist_itor_t *func, void *arg, dmu_tx_t *tx)
{
	bplist_entry_t *bpe;

	mutex_enter(&bpl->bpl_lock);
	while (bpe = list_head(&bpl->bpl_list)) {
		bplist_iterate_last_removed = bpe;
		list_remove(&bpl->bpl_list, bpe);
		mutex_exit(&bpl->bpl_lock);
		func(arg, &bpe->bpe_blk, tx);
		kmem_free(bpe, sizeof (*bpe));
		mutex_enter(&bpl->bpl_lock);
	}
	mutex_exit(&bpl->bpl_lock);
}
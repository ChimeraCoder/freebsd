
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/zfs_context.h>
#include <sys/avl.h>
#include <sys/unique.h>

static avl_tree_t unique_avl;
static kmutex_t unique_mtx;

typedef struct unique {
	avl_node_t un_link;
	uint64_t un_value;
} unique_t;

#define	UNIQUE_MASK ((1ULL << UNIQUE_BITS) - 1)

static int
unique_compare(const void *a, const void *b)
{
	const unique_t *una = a;
	const unique_t *unb = b;

	if (una->un_value < unb->un_value)
		return (-1);
	if (una->un_value > unb->un_value)
		return (+1);
	return (0);
}

void
unique_init(void)
{
	avl_create(&unique_avl, unique_compare,
	    sizeof (unique_t), offsetof(unique_t, un_link));
	mutex_init(&unique_mtx, NULL, MUTEX_DEFAULT, NULL);
}

void
unique_fini(void)
{
	avl_destroy(&unique_avl);
	mutex_destroy(&unique_mtx);
}

uint64_t
unique_create(void)
{
	uint64_t value = unique_insert(0);
	unique_remove(value);
	return (value);
}

uint64_t
unique_insert(uint64_t value)
{
	avl_index_t idx;
	unique_t *un = kmem_alloc(sizeof (unique_t), KM_SLEEP);

	un->un_value = value;

	mutex_enter(&unique_mtx);
	while (un->un_value == 0 || un->un_value & ~UNIQUE_MASK ||
	    avl_find(&unique_avl, un, &idx)) {
		mutex_exit(&unique_mtx);
		(void) random_get_pseudo_bytes((void*)&un->un_value,
		    sizeof (un->un_value));
		un->un_value &= UNIQUE_MASK;
		mutex_enter(&unique_mtx);
	}

	avl_insert(&unique_avl, un, idx);
	mutex_exit(&unique_mtx);

	return (un->un_value);
}

void
unique_remove(uint64_t value)
{
	unique_t un_tofind;
	unique_t *un;

	un_tofind.un_value = value;
	mutex_enter(&unique_mtx);
	un = avl_find(&unique_avl, &un_tofind, NULL);
	if (un != NULL) {
		avl_remove(&unique_avl, un);
		kmem_free(un, sizeof (unique_t));
	}
	mutex_exit(&unique_mtx);
}

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
 * Simple open hash tab implementation.
 *
 * Authors:
 * Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */

#include "dev/drm/drmP.h"
#include "dev/drm/drm_hashtab.h"

#include <sys/hash.h>

int drm_ht_create(struct drm_open_hash *ht, unsigned int order)
{
	ht->size = 1 << order;
	ht->order = order;
	ht->table = NULL;
	ht->table = hashinit_flags(ht->size, DRM_MEM_HASHTAB, &ht->mask,
	    HASH_NOWAIT);
	if (!ht->table) {
		DRM_ERROR("Out of memory for hash table\n");
		return -ENOMEM;
	}
	return 0;
}

void drm_ht_verbose_list(struct drm_open_hash *ht, unsigned long key)
{
	struct drm_hash_item *entry;
	struct drm_hash_item_list *h_list;
	unsigned int hashed_key;
	int count = 0;

	hashed_key = hash32_buf(&key, sizeof(key), ht->order);
	DRM_DEBUG("Key is 0x%08lx, Hashed key is 0x%08x\n", key, hashed_key);
	h_list = &ht->table[hashed_key & ht->mask];
	LIST_FOREACH(entry, h_list, head)
		DRM_DEBUG("count %d, key: 0x%08lx\n", count++, entry->key);
}

static struct drm_hash_item *
drm_ht_find_key(struct drm_open_hash *ht, unsigned long key)
{
	struct drm_hash_item *entry;
	struct drm_hash_item_list *h_list;
	unsigned int hashed_key;

	hashed_key = hash32_buf(&key, sizeof(key), ht->order);
	h_list = &ht->table[hashed_key & ht->mask];
	LIST_FOREACH(entry, h_list, head) {
		if (entry->key == key)
			return entry;
		if (entry->key > key)
			break;
	}
	return NULL;
}


int drm_ht_insert_item(struct drm_open_hash *ht, struct drm_hash_item *item)
{
	struct drm_hash_item *entry, *parent;
	struct drm_hash_item_list *h_list;
	unsigned int hashed_key;
	unsigned long key = item->key;

	hashed_key = hash32_buf(&key, sizeof(key), ht->order);
	h_list = &ht->table[hashed_key & ht->mask];
	parent = NULL;
	LIST_FOREACH(entry, h_list, head) {
		if (entry->key == key)
			return -EINVAL;
		if (entry->key > key)
			break;
		parent = entry;
	}
	if (parent) {
		LIST_INSERT_AFTER(parent, item, head);
	} else {
		LIST_INSERT_HEAD(h_list, item, head);
	}
	return 0;
}

/*
 * Just insert an item and return any "bits" bit key that hasn't been
 * used before.
 */
int drm_ht_just_insert_please(struct drm_open_hash *ht, struct drm_hash_item *item,
			      unsigned long seed, int bits, int shift,
			      unsigned long add)
{
	int ret;
	unsigned long mask = (1 << bits) - 1;
	unsigned long first, unshifted_key = 0;

	unshifted_key = hash32_buf(&seed, sizeof(seed), unshifted_key);
	first = unshifted_key;
	do {
		item->key = (unshifted_key << shift) + add;
		ret = drm_ht_insert_item(ht, item);
		if (ret)
			unshifted_key = (unshifted_key + 1) & mask;
	} while(ret && (unshifted_key != first));

	if (ret) {
		DRM_ERROR("Available key bit space exhausted\n");
		return -EINVAL;
	}
	return 0;
}

int drm_ht_find_item(struct drm_open_hash *ht, unsigned long key,
		     struct drm_hash_item **item)
{
	struct drm_hash_item *entry;

	entry = drm_ht_find_key(ht, key);
	if (!entry)
		return -EINVAL;

	*item = entry;
	return 0;
}

int drm_ht_remove_key(struct drm_open_hash *ht, unsigned long key)
{
	struct drm_hash_item *entry;

	entry = drm_ht_find_key(ht, key);
	if (entry) {
		LIST_REMOVE(entry, head);
		return 0;
	}
	return -EINVAL;
}

int drm_ht_remove_item(struct drm_open_hash *ht, struct drm_hash_item *item)
{
	LIST_REMOVE(item, head);
	return 0;
}

void drm_ht_remove(struct drm_open_hash *ht)
{
	if (ht->table) {
		hashdestroy(ht->table, DRM_MEM_HASHTAB, ht->mask);
		ht->table = NULL;
	}
}

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

#include <dev/drm2/drmP.h>
#include <dev/drm2/ttm/ttm_execbuf_util.h>
#include <dev/drm2/ttm/ttm_bo_driver.h>
#include <dev/drm2/ttm/ttm_placement.h>

static void ttm_eu_backoff_reservation_locked(struct list_head *list)
{
	struct ttm_validate_buffer *entry;

	list_for_each_entry(entry, list, head) {
		struct ttm_buffer_object *bo = entry->bo;
		if (!entry->reserved)
			continue;

		if (entry->removed) {
			ttm_bo_add_to_lru(bo);
			entry->removed = false;

		}
		entry->reserved = false;
		atomic_set(&bo->reserved, 0);
		wakeup(bo);
	}
}

static void ttm_eu_del_from_lru_locked(struct list_head *list)
{
	struct ttm_validate_buffer *entry;

	list_for_each_entry(entry, list, head) {
		struct ttm_buffer_object *bo = entry->bo;
		if (!entry->reserved)
			continue;

		if (!entry->removed) {
			entry->put_count = ttm_bo_del_from_lru(bo);
			entry->removed = true;
		}
	}
}

static void ttm_eu_list_ref_sub(struct list_head *list)
{
	struct ttm_validate_buffer *entry;

	list_for_each_entry(entry, list, head) {
		struct ttm_buffer_object *bo = entry->bo;

		if (entry->put_count) {
			ttm_bo_list_ref_sub(bo, entry->put_count, true);
			entry->put_count = 0;
		}
	}
}

static int ttm_eu_wait_unreserved_locked(struct list_head *list,
					 struct ttm_buffer_object *bo)
{
	int ret;

	ttm_eu_del_from_lru_locked(list);
	ret = ttm_bo_wait_unreserved_locked(bo, true);
	if (unlikely(ret != 0))
		ttm_eu_backoff_reservation_locked(list);
	return ret;
}


void ttm_eu_backoff_reservation(struct list_head *list)
{
	struct ttm_validate_buffer *entry;
	struct ttm_bo_global *glob;

	if (list_empty(list))
		return;

	entry = list_first_entry(list, struct ttm_validate_buffer, head);
	glob = entry->bo->glob;
	mtx_lock(&glob->lru_lock);
	ttm_eu_backoff_reservation_locked(list);
	mtx_unlock(&glob->lru_lock);
}

/*
 * Reserve buffers for validation.
 *
 * If a buffer in the list is marked for CPU access, we back off and
 * wait for that buffer to become free for GPU access.
 *
 * If a buffer is reserved for another validation, the validator with
 * the highest validation sequence backs off and waits for that buffer
 * to become unreserved. This prevents deadlocks when validating multiple
 * buffers in different orders.
 */

int ttm_eu_reserve_buffers(struct list_head *list)
{
	struct ttm_bo_global *glob;
	struct ttm_validate_buffer *entry;
	int ret;
	uint32_t val_seq;

	if (list_empty(list))
		return 0;

	list_for_each_entry(entry, list, head) {
		entry->reserved = false;
		entry->put_count = 0;
		entry->removed = false;
	}

	entry = list_first_entry(list, struct ttm_validate_buffer, head);
	glob = entry->bo->glob;

	mtx_lock(&glob->lru_lock);
retry_locked:
	val_seq = entry->bo->bdev->val_seq++;

	list_for_each_entry(entry, list, head) {
		struct ttm_buffer_object *bo = entry->bo;

retry_this_bo:
		ret = ttm_bo_reserve_locked(bo, true, true, true, val_seq);
		switch (ret) {
		case 0:
			break;
		case -EBUSY:
			ret = ttm_eu_wait_unreserved_locked(list, bo);
			if (unlikely(ret != 0)) {
				mtx_unlock(&glob->lru_lock);
				ttm_eu_list_ref_sub(list);
				return ret;
			}
			goto retry_this_bo;
		case -EAGAIN:
			ttm_eu_backoff_reservation_locked(list);
			ttm_eu_list_ref_sub(list);
			ret = ttm_bo_wait_unreserved_locked(bo, true);
			if (unlikely(ret != 0)) {
				mtx_unlock(&glob->lru_lock);
				return ret;
			}
			goto retry_locked;
		default:
			ttm_eu_backoff_reservation_locked(list);
			mtx_unlock(&glob->lru_lock);
			ttm_eu_list_ref_sub(list);
			return ret;
		}

		entry->reserved = true;
		if (unlikely(atomic_read(&bo->cpu_writers) > 0)) {
			ttm_eu_backoff_reservation_locked(list);
			mtx_unlock(&glob->lru_lock);
			ttm_eu_list_ref_sub(list);
			return -EBUSY;
		}
	}

	ttm_eu_del_from_lru_locked(list);
	mtx_unlock(&glob->lru_lock);
	ttm_eu_list_ref_sub(list);

	return 0;
}

void ttm_eu_fence_buffer_objects(struct list_head *list, void *sync_obj)
{
	struct ttm_validate_buffer *entry;
	struct ttm_buffer_object *bo;
	struct ttm_bo_global *glob;
	struct ttm_bo_device *bdev;
	struct ttm_bo_driver *driver;

	if (list_empty(list))
		return;

	bo = list_first_entry(list, struct ttm_validate_buffer, head)->bo;
	bdev = bo->bdev;
	driver = bdev->driver;
	glob = bo->glob;

	mtx_lock(&glob->lru_lock);
	mtx_lock(&bdev->fence_lock);

	list_for_each_entry(entry, list, head) {
		bo = entry->bo;
		entry->old_sync_obj = bo->sync_obj;
		bo->sync_obj = driver->sync_obj_ref(sync_obj);
		ttm_bo_unreserve_locked(bo);
		entry->reserved = false;
	}
	mtx_unlock(&bdev->fence_lock);
	mtx_unlock(&glob->lru_lock);

	list_for_each_entry(entry, list, head) {
		if (entry->old_sync_obj)
			driver->sync_obj_unref(&entry->old_sync_obj);
	}
}
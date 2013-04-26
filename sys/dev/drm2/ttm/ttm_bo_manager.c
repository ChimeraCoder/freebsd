
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
#include <dev/drm2/ttm/ttm_module.h>
#include <dev/drm2/ttm/ttm_bo_driver.h>
#include <dev/drm2/ttm/ttm_placement.h>
#include <dev/drm2/drm_mm.h>

/**
 * Currently we use a spinlock for the lock, but a mutex *may* be
 * more appropriate to reduce scheduling latency if the range manager
 * ends up with very fragmented allocation patterns.
 */

struct ttm_range_manager {
	struct drm_mm mm;
	struct mtx lock;
};

MALLOC_DEFINE(M_TTM_RMAN, "ttm_rman", "TTM Range Manager");

static int ttm_bo_man_get_node(struct ttm_mem_type_manager *man,
			       struct ttm_buffer_object *bo,
			       struct ttm_placement *placement,
			       struct ttm_mem_reg *mem)
{
	struct ttm_range_manager *rman = (struct ttm_range_manager *) man->priv;
	struct drm_mm *mm = &rman->mm;
	struct drm_mm_node *node = NULL;
	unsigned long lpfn;
	int ret;

	lpfn = placement->lpfn;
	if (!lpfn)
		lpfn = man->size;
	do {
		ret = drm_mm_pre_get(mm);
		if (unlikely(ret))
			return ret;

		mtx_lock(&rman->lock);
		node = drm_mm_search_free_in_range(mm,
					mem->num_pages, mem->page_alignment,
					placement->fpfn, lpfn, 1);
		if (unlikely(node == NULL)) {
			mtx_unlock(&rman->lock);
			return 0;
		}
		node = drm_mm_get_block_atomic_range(node, mem->num_pages,
						     mem->page_alignment,
						     placement->fpfn,
						     lpfn);
		mtx_unlock(&rman->lock);
	} while (node == NULL);

	mem->mm_node = node;
	mem->start = node->start;
	return 0;
}

static void ttm_bo_man_put_node(struct ttm_mem_type_manager *man,
				struct ttm_mem_reg *mem)
{
	struct ttm_range_manager *rman = (struct ttm_range_manager *) man->priv;

	if (mem->mm_node) {
		mtx_lock(&rman->lock);
		drm_mm_put_block(mem->mm_node);
		mtx_unlock(&rman->lock);
		mem->mm_node = NULL;
	}
}

static int ttm_bo_man_init(struct ttm_mem_type_manager *man,
			   unsigned long p_size)
{
	struct ttm_range_manager *rman;
	int ret;

	rman = malloc(sizeof(*rman), M_TTM_RMAN, M_ZERO | M_WAITOK);
	ret = drm_mm_init(&rman->mm, 0, p_size);
	if (ret) {
		free(rman, M_TTM_RMAN);
		return ret;
	}

	mtx_init(&rman->lock, "ttmrman", NULL, MTX_DEF);
	man->priv = rman;
	return 0;
}

static int ttm_bo_man_takedown(struct ttm_mem_type_manager *man)
{
	struct ttm_range_manager *rman = (struct ttm_range_manager *) man->priv;
	struct drm_mm *mm = &rman->mm;

	mtx_lock(&rman->lock);
	if (drm_mm_clean(mm)) {
		drm_mm_takedown(mm);
		mtx_unlock(&rman->lock);
		mtx_destroy(&rman->lock);
		free(rman, M_TTM_RMAN);
		man->priv = NULL;
		return 0;
	}
	mtx_unlock(&rman->lock);
	return -EBUSY;
}

static void ttm_bo_man_debug(struct ttm_mem_type_manager *man,
			     const char *prefix)
{
	struct ttm_range_manager *rman = (struct ttm_range_manager *) man->priv;

	mtx_lock(&rman->lock);
	drm_mm_debug_table(&rman->mm, prefix);
	mtx_unlock(&rman->lock);
}

const struct ttm_mem_type_manager_func ttm_bo_manager_func = {
	ttm_bo_man_init,
	ttm_bo_man_takedown,
	ttm_bo_man_get_node,
	ttm_bo_man_put_node,
	ttm_bo_man_debug
};
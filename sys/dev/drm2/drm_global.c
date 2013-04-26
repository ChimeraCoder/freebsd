
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
#include <dev/drm2/drm_global.h>

MALLOC_DEFINE(M_DRM_GLOBAL, "drm_global", "DRM Global Items");

struct drm_global_item {
	struct sx mutex;
	void *object;
	int refcount;
};

static struct drm_global_item glob[DRM_GLOBAL_NUM];

void drm_global_init(void)
{
	int i;

	for (i = 0; i < DRM_GLOBAL_NUM; ++i) {
		struct drm_global_item *item = &glob[i];
		sx_init(&item->mutex, "drmgi");
		item->object = NULL;
		item->refcount = 0;
	}
}

void drm_global_release(void)
{
	int i;
	for (i = 0; i < DRM_GLOBAL_NUM; ++i) {
		struct drm_global_item *item = &glob[i];
		MPASS(item->object == NULL);
		MPASS(item->refcount == 0);
		sx_destroy(&item->mutex);
	}
}

int drm_global_item_ref(struct drm_global_reference *ref)
{
	int ret;
	struct drm_global_item *item = &glob[ref->global_type];
	void *object;

	sx_xlock(&item->mutex);
	if (item->refcount == 0) {
		item->object = malloc(ref->size, M_DRM_GLOBAL,
		    M_WAITOK | M_ZERO);

		ref->object = item->object;
		ret = ref->init(ref);
		if (unlikely(ret != 0))
			goto out_err;

	}
	++item->refcount;
	ref->object = item->object;
	object = item->object;
	sx_xunlock(&item->mutex);
	return 0;
out_err:
	sx_xunlock(&item->mutex);
	item->object = NULL;
	return ret;
}

void drm_global_item_unref(struct drm_global_reference *ref)
{
	struct drm_global_item *item = &glob[ref->global_type];

	sx_xlock(&item->mutex);
	MPASS(item->refcount != 0);
	MPASS(ref->object == item->object);
	if (--item->refcount == 0) {
		ref->release(ref);
		free(item->object, M_DRM_GLOBAL);
		item->object = NULL;
	}
	sx_xunlock(&item->mutex);
}
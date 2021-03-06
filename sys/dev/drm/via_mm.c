
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
 * Authors: Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "dev/drm/drmP.h"
#include "dev/drm/via_drm.h"
#include "dev/drm/via_drv.h"
#include "dev/drm/drm_sman.h"

#define VIA_MM_ALIGN_SHIFT 4
#define VIA_MM_ALIGN_MASK ( (1 << VIA_MM_ALIGN_SHIFT) - 1)

int via_agp_init(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_via_agp_t *agp = data;
	drm_via_private_t *dev_priv = (drm_via_private_t *) dev->dev_private;
	int ret;

	ret = drm_sman_set_range(&dev_priv->sman, VIA_MEM_AGP, 0,
				 agp->size >> VIA_MM_ALIGN_SHIFT);
	if (ret) {
		DRM_ERROR("AGP memory manager initialisation error\n");
		return ret;
	}

	dev_priv->agp_initialized = 1;
	dev_priv->agp_offset = agp->offset;

	DRM_DEBUG("offset = %u, size = %u\n", agp->offset, agp->size);
	return 0;
}

int via_fb_init(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_via_fb_t *fb = data;
	drm_via_private_t *dev_priv = (drm_via_private_t *) dev->dev_private;
	int ret;

	ret = drm_sman_set_range(&dev_priv->sman, VIA_MEM_VIDEO, 0,
				 fb->size >> VIA_MM_ALIGN_SHIFT);
	if (ret) {
		DRM_ERROR("VRAM memory manager initialisation error\n");
		return ret;
	}

	dev_priv->vram_initialized = 1;
	dev_priv->vram_offset = fb->offset;

	DRM_DEBUG("offset = %u, size = %u\n", fb->offset, fb->size);

	return 0;

}

int via_final_context(struct drm_device *dev, int context)
{
	drm_via_private_t *dev_priv = (drm_via_private_t *) dev->dev_private;

	via_release_futex(dev_priv, context);

#ifdef __linux__
	/* Linux specific until context tracking code gets ported to BSD */
	/* Last context, perform cleanup */
	if (dev->ctx_count == 1 && dev->dev_private) {
		DRM_DEBUG("Last Context\n");
		drm_irq_uninstall(dev);
		via_cleanup_futex(dev_priv);
		via_do_cleanup_map(dev);
	}
#endif
	return 1;
}

void via_lastclose(struct drm_device *dev)
{
	drm_via_private_t *dev_priv = (drm_via_private_t *) dev->dev_private;

	if (!dev_priv)
		return;

	drm_sman_cleanup(&dev_priv->sman);
	dev_priv->vram_initialized = 0;
	dev_priv->agp_initialized = 0;
}

int via_mem_alloc(struct drm_device *dev, void *data,
		  struct drm_file *file_priv)
{
	drm_via_mem_t *mem = data;
	int retval = 0;
	struct drm_memblock_item *item;
	drm_via_private_t *dev_priv = (drm_via_private_t *) dev->dev_private;
	unsigned long tmpSize;

	if (mem->type > VIA_MEM_AGP) {
		DRM_ERROR("Unknown memory type allocation\n");
		return -EINVAL;
	}
	if (0 == ((mem->type == VIA_MEM_VIDEO) ? dev_priv->vram_initialized :
		      dev_priv->agp_initialized)) {
		DRM_ERROR
		    ("Attempt to allocate from uninitialized memory manager.\n");
		return -EINVAL;
	}

	tmpSize = (mem->size + VIA_MM_ALIGN_MASK) >> VIA_MM_ALIGN_SHIFT;
	item = drm_sman_alloc(&dev_priv->sman, mem->type, tmpSize, 0,
			      (unsigned long)file_priv);
	if (item) {
		mem->offset = ((mem->type == VIA_MEM_VIDEO) ?
			      dev_priv->vram_offset : dev_priv->agp_offset) +
		    (item->mm->
		     offset(item->mm, item->mm_info) << VIA_MM_ALIGN_SHIFT);
		mem->index = item->user_hash.key;
	} else {
		mem->offset = 0;
		mem->size = 0;
		mem->index = 0;
		DRM_DEBUG("Video memory allocation failed\n");
		retval = -ENOMEM;
	}

	return retval;
}

int via_mem_free(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	drm_via_private_t *dev_priv = dev->dev_private;
	drm_via_mem_t *mem = data;
	int ret;

	ret = drm_sman_free_key(&dev_priv->sman, mem->index);
	DRM_DEBUG("free = 0x%lx\n", mem->index);

	return ret;
}


void via_reclaim_buffers_locked(struct drm_device * dev,
				struct drm_file *file_priv)
{
	drm_via_private_t *dev_priv = dev->dev_private;

	if (drm_sman_owner_clean(&dev_priv->sman, (unsigned long)file_priv))
		return;

	if (dev->driver->dma_quiescent)
		dev->driver->dma_quiescent(dev);

	drm_sman_owner_cleanup(&dev_priv->sman, (unsigned long)file_priv);

	return;
}
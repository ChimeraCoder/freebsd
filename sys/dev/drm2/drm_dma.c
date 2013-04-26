
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

/** @file drm_dma.c
 * Support code for DMA buffer management.
 *
 * The implementation used to be significantly more complicated, but the
 * complexity has been moved into the drivers as different buffer management
 * schemes evolved.
 */

#include <dev/drm2/drmP.h>

int drm_dma_setup(struct drm_device *dev)
{

	dev->dma = malloc(sizeof(*dev->dma), DRM_MEM_DRIVER, M_NOWAIT | M_ZERO);
	if (dev->dma == NULL)
		return ENOMEM;

	DRM_SPININIT(&dev->dma_lock, "drmdma");

	return 0;
}

void drm_dma_takedown(struct drm_device *dev)
{
	drm_device_dma_t  *dma = dev->dma;
	int		  i, j;

	if (dma == NULL)
		return;

	/* Clear dma buffers */
	for (i = 0; i <= DRM_MAX_ORDER; i++) {
		if (dma->bufs[i].seg_count) {
			DRM_DEBUG("order %d: buf_count = %d,"
			    " seg_count = %d\n", i, dma->bufs[i].buf_count,
			    dma->bufs[i].seg_count);
			for (j = 0; j < dma->bufs[i].seg_count; j++) {
				drm_pci_free(dev, dma->bufs[i].seglist[j]);
			}
			free(dma->bufs[i].seglist, DRM_MEM_SEGS);
		}

	   	if (dma->bufs[i].buf_count) {
		   	for (j = 0; j < dma->bufs[i].buf_count; j++) {
				free(dma->bufs[i].buflist[j].dev_private,
				    DRM_MEM_BUFS);
			}
		   	free(dma->bufs[i].buflist, DRM_MEM_BUFS);
		}
	}

	free(dma->buflist, DRM_MEM_BUFS);
	free(dma->pagelist, DRM_MEM_PAGES);
	free(dev->dma, DRM_MEM_DRIVER);
	dev->dma = NULL;
	DRM_SPINUNINIT(&dev->dma_lock);
}


void drm_free_buffer(struct drm_device *dev, drm_buf_t *buf)
{
	if (!buf)
		return;

	buf->pending  = 0;
	buf->file_priv= NULL;
	buf->used     = 0;
}

void drm_reclaim_buffers(struct drm_device *dev, struct drm_file *file_priv)
{
	drm_device_dma_t *dma = dev->dma;
	int		 i;

	if (!dma)
		return;

	for (i = 0; i < dma->buf_count; i++) {
		if (dma->buflist[i]->file_priv == file_priv) {
			switch (dma->buflist[i]->list) {
			case DRM_LIST_NONE:
				drm_free_buffer(dev, dma->buflist[i]);
				break;
			case DRM_LIST_WAIT:
				dma->buflist[i]->list = DRM_LIST_RECLAIM;
				break;
			default:
				/* Buffer already on hardware. */
				break;
			}
		}
	}
}

/* Call into the driver-specific DMA handler */
int drm_dma(struct drm_device *dev, void *data, struct drm_file *file_priv)
{

	if (dev->driver->dma_ioctl) {
		/* shared code returns -errno */
		return -dev->driver->dma_ioctl(dev, data, file_priv);
	} else {
		DRM_DEBUG("DMA ioctl on driver with no dma handler\n");
		return EINVAL;
	}
}
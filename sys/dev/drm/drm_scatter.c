
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

/** @file drm_scatter.c
 * Allocation of memory for scatter-gather mappings by the graphics chip.
 * The memory allocated here is then made into an aperture in the card
 * by mapping the pages into the GART.
 */

#include "dev/drm/drmP.h"

int
drm_sg_alloc(struct drm_device *dev, struct drm_scatter_gather *request)
{
	struct drm_sg_mem *entry;
	vm_size_t size;
	vm_pindex_t pindex;

	if (dev->sg)
		return EINVAL;

	DRM_DEBUG("request size=%ld\n", request->size);

	entry = malloc(sizeof(*entry), DRM_MEM_DRIVER, M_WAITOK | M_ZERO);

	size = round_page(request->size);
	entry->pages = OFF_TO_IDX(size);
	entry->busaddr = malloc(entry->pages * sizeof(*entry->busaddr),
	    DRM_MEM_SGLISTS, M_WAITOK | M_ZERO);

	entry->vaddr = kmem_alloc_attr(kernel_map, size, M_WAITOK | M_ZERO,
	    0, BUS_SPACE_MAXADDR_32BIT, VM_MEMATTR_WRITE_COMBINING);
	if (entry->vaddr == 0) {
		drm_sg_cleanup(entry);
		return (ENOMEM);
	}

	for(pindex = 0; pindex < entry->pages; pindex++) {
		entry->busaddr[pindex] =
		    vtophys(entry->vaddr + IDX_TO_OFF(pindex));
	}

	DRM_LOCK();
	if (dev->sg) {
		DRM_UNLOCK();
		drm_sg_cleanup(entry);
		return (EINVAL);
	}
	dev->sg = entry;
	DRM_UNLOCK();

	request->handle = entry->vaddr;

	DRM_DEBUG("allocated %ju pages @ 0x%08zx, contents=%08lx\n",
	    entry->pages, entry->vaddr, *(unsigned long *)entry->vaddr);

	return (0);
}

int
drm_sg_alloc_ioctl(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_scatter_gather *request = data;

	DRM_DEBUG("\n");

	return (drm_sg_alloc(dev, request));
}

void
drm_sg_cleanup(struct drm_sg_mem *entry)
{
	if (entry == NULL)
		return;

	if (entry->vaddr != 0)
		kmem_free(kernel_map, entry->vaddr, IDX_TO_OFF(entry->pages));

	free(entry->busaddr, DRM_MEM_SGLISTS);
	free(entry, DRM_MEM_DRIVER);

	return;
}

int
drm_sg_free(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_scatter_gather *request = data;
	struct drm_sg_mem *entry;

	DRM_LOCK();
	entry = dev->sg;
	dev->sg = NULL;
	DRM_UNLOCK();

	if (!entry || entry->vaddr != request->handle)
		return (EINVAL);

	DRM_DEBUG("free 0x%zx\n", entry->vaddr);

	drm_sg_cleanup(entry);

	return (0);
}

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

/** @file drm_vm.c
 * Support code for mmaping of DRM maps.
 */

#include <dev/drm2/drmP.h>
#include <dev/drm2/drm.h>

int
drm_mmap(struct cdev *kdev, vm_ooffset_t offset, vm_paddr_t *paddr,
    int prot, vm_memattr_t *memattr)
{
	struct drm_device *dev = drm_get_device_from_kdev(kdev);
	struct drm_file *file_priv = NULL;
	drm_local_map_t *map;
	enum drm_map_type type;
	vm_paddr_t phys;
	int error;

	/* d_mmap gets called twice, we can only reference file_priv during
	 * the first call.  We need to assume that if error is EBADF the
	 * call was succesful and the client is authenticated.
	 */
	error = devfs_get_cdevpriv((void **)&file_priv);
	if (error == ENOENT) {
		DRM_ERROR("Could not find authenticator!\n");
		return EINVAL;
	}

	if (file_priv && !file_priv->authenticated)
		return EACCES;

	DRM_DEBUG("called with offset %016jx\n", offset);
	if (dev->dma && offset < ptoa(dev->dma->page_count)) {
		drm_device_dma_t *dma = dev->dma;

		DRM_SPINLOCK(&dev->dma_lock);

		if (dma->pagelist != NULL) {
			unsigned long page = offset >> PAGE_SHIFT;
			unsigned long phys = dma->pagelist[page];

			DRM_SPINUNLOCK(&dev->dma_lock);
			*paddr = phys;
			return 0;
		} else {
			DRM_SPINUNLOCK(&dev->dma_lock);
			return -1;
		}
	}

	/* A sequential search of a linked list is
	   fine here because: 1) there will only be
	   about 5-10 entries in the list and, 2) a
	   DRI client only has to do this mapping
	   once, so it doesn't have to be optimized
	   for performance, even if the list was a
	   bit longer.
	*/
	DRM_LOCK(dev);
	TAILQ_FOREACH(map, &dev->maplist, link) {
		if (offset >> DRM_MAP_HANDLE_SHIFT ==
		    (unsigned long)map->handle >> DRM_MAP_HANDLE_SHIFT)
			break;
	}

	if (map == NULL) {
		DRM_DEBUG("Can't find map, request offset = %016jx\n", offset);
		TAILQ_FOREACH(map, &dev->maplist, link) {
			DRM_DEBUG("map offset = %016lx, handle = %016lx\n",
			    map->offset, (unsigned long)map->handle);
		}
		DRM_UNLOCK(dev);
		return -1;
	}
	if (((map->flags & _DRM_RESTRICTED) && !DRM_SUSER(DRM_CURPROC))) {
		DRM_UNLOCK(dev);
		DRM_DEBUG("restricted map\n");
		return -1;
	}
	type = map->type;
	DRM_UNLOCK(dev);

	offset = offset & ((1ULL << DRM_MAP_HANDLE_SHIFT) - 1);

	switch (type) {
	case _DRM_FRAME_BUFFER:
	case _DRM_AGP:
		*memattr = VM_MEMATTR_WRITE_COMBINING;
		/* FALLTHROUGH */
	case _DRM_REGISTERS:
		phys = map->offset + offset;
		break;
	case _DRM_SCATTER_GATHER:
		*memattr = VM_MEMATTR_WRITE_COMBINING;
		/* FALLTHROUGH */
	case _DRM_CONSISTENT:
	case _DRM_SHM:
		phys = vtophys((char *)map->virtual + offset);
		break;
	default:
		DRM_ERROR("bad map type %d\n", type);
		return -1;	/* This should never happen. */
	}

	*paddr = phys;
	return 0;
}
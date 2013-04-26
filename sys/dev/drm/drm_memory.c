
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

/** @file drm_memory.c
 * Wrappers for kernel memory allocation routines, and MTRR management support.
 *
 * This file previously implemented a memory consumption tracking system using
 * the "area" argument for various different types of allocations, but that
 * has been stripped out for now.
 */

#include "dev/drm/drmP.h"

MALLOC_DEFINE(DRM_MEM_DMA, "drm_dma", "DRM DMA Data Structures");
MALLOC_DEFINE(DRM_MEM_SAREA, "drm_sarea", "DRM SAREA Data Structures");
MALLOC_DEFINE(DRM_MEM_DRIVER, "drm_driver", "DRM DRIVER Data Structures");
MALLOC_DEFINE(DRM_MEM_MAGIC, "drm_magic", "DRM MAGIC Data Structures");
MALLOC_DEFINE(DRM_MEM_IOCTLS, "drm_ioctls", "DRM IOCTL Data Structures");
MALLOC_DEFINE(DRM_MEM_MAPS, "drm_maps", "DRM MAP Data Structures");
MALLOC_DEFINE(DRM_MEM_BUFS, "drm_bufs", "DRM BUFFER Data Structures");
MALLOC_DEFINE(DRM_MEM_SEGS, "drm_segs", "DRM SEGMENTS Data Structures");
MALLOC_DEFINE(DRM_MEM_PAGES, "drm_pages", "DRM PAGES Data Structures");
MALLOC_DEFINE(DRM_MEM_FILES, "drm_files", "DRM FILE Data Structures");
MALLOC_DEFINE(DRM_MEM_QUEUES, "drm_queues", "DRM QUEUE Data Structures");
MALLOC_DEFINE(DRM_MEM_CMDS, "drm_cmds", "DRM COMMAND Data Structures");
MALLOC_DEFINE(DRM_MEM_MAPPINGS, "drm_mapping", "DRM MAPPING Data Structures");
MALLOC_DEFINE(DRM_MEM_BUFLISTS, "drm_buflists", "DRM BUFLISTS Data Structures");
MALLOC_DEFINE(DRM_MEM_AGPLISTS, "drm_agplists", "DRM AGPLISTS Data Structures");
MALLOC_DEFINE(DRM_MEM_CTXBITMAP, "drm_ctxbitmap",
    "DRM CTXBITMAP Data Structures");
MALLOC_DEFINE(DRM_MEM_SGLISTS, "drm_sglists", "DRM SGLISTS Data Structures");
MALLOC_DEFINE(DRM_MEM_DRAWABLE, "drm_drawable", "DRM DRAWABLE Data Structures");
MALLOC_DEFINE(DRM_MEM_MM, "drm_sman", "DRM MEMORY MANAGER Data Structures");
MALLOC_DEFINE(DRM_MEM_HASHTAB, "drm_hashtab", "DRM HASHTABLE Data Structures");

void drm_mem_init(void)
{
}

void drm_mem_uninit(void)
{
}

void *drm_ioremap_wc(struct drm_device *dev, drm_local_map_t *map)
{
	return pmap_mapdev_attr(map->offset, map->size, VM_MEMATTR_WRITE_COMBINING);
}

void *drm_ioremap(struct drm_device *dev, drm_local_map_t *map)
{
	return pmap_mapdev(map->offset, map->size);
}

void drm_ioremapfree(drm_local_map_t *map)
{
	pmap_unmapdev((vm_offset_t) map->virtual, map->size);
}

int
drm_mtrr_add(unsigned long offset, size_t size, int flags)
{
	int act;
	struct mem_range_desc mrdesc;

	mrdesc.mr_base = offset;
	mrdesc.mr_len = size;
	mrdesc.mr_flags = flags;
	act = MEMRANGE_SET_UPDATE;
	strlcpy(mrdesc.mr_owner, "drm", sizeof(mrdesc.mr_owner));
	return mem_range_attr_set(&mrdesc, &act);
}

int
drm_mtrr_del(int __unused handle, unsigned long offset, size_t size, int flags)
{
	int act;
	struct mem_range_desc mrdesc;

	mrdesc.mr_base = offset;
	mrdesc.mr_len = size;
	mrdesc.mr_flags = flags;
	act = MEMRANGE_SET_REMOVE;
	strlcpy(mrdesc.mr_owner, "drm", sizeof(mrdesc.mr_owner));
	return mem_range_attr_set(&mrdesc, &act);
}

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

/** @file drm_ioctl.c
 * Varios minor DRM ioctls not applicable to other files, such as versioning
 * information and reporting DRM information to userland.
 */

#include "dev/drm/drmP.h"

/*
 * Beginning in revision 1.1 of the DRM interface, getunique will return
 * a unique in the form pci:oooo:bb:dd.f (o=domain, b=bus, d=device, f=function)
 * before setunique has been called.  The format for the bus-specific part of
 * the unique is not defined for any other bus.
 */
int drm_getunique(struct drm_device *dev, void *data,
		  struct drm_file *file_priv)
{
	struct drm_unique *u = data;

	if (u->unique_len >= dev->unique_len) {
		if (DRM_COPY_TO_USER(u->unique, dev->unique, dev->unique_len))
			return EFAULT;
	}
	u->unique_len = dev->unique_len;

	return 0;
}

/* Deprecated in DRM version 1.1, and will return EBUSY when setversion has
 * requested version 1.1 or greater.
 */
int drm_setunique(struct drm_device *dev, void *data,
		  struct drm_file *file_priv)
{
	struct drm_unique *u = data;
	int domain, bus, slot, func, ret;
	char *busid;

	/* Check and copy in the submitted Bus ID */
	if (!u->unique_len || u->unique_len > 1024)
		return EINVAL;

	busid = malloc(u->unique_len + 1, DRM_MEM_DRIVER, M_WAITOK);
	if (busid == NULL)
		return ENOMEM;

	if (DRM_COPY_FROM_USER(busid, u->unique, u->unique_len)) {
		free(busid, DRM_MEM_DRIVER);
		return EFAULT;
	}
	busid[u->unique_len] = '\0';

	/* Return error if the busid submitted doesn't match the device's actual
	 * busid.
	 */
	ret = sscanf(busid, "PCI:%d:%d:%d", &bus, &slot, &func);
	if (ret != 3) {
		free(busid, DRM_MEM_DRIVER);
		return EINVAL;
	}
	domain = bus >> 8;
	bus &= 0xff;
	
	if ((domain != dev->pci_domain) ||
	    (bus != dev->pci_bus) ||
	    (slot != dev->pci_slot) ||
	    (func != dev->pci_func)) {
		free(busid, DRM_MEM_DRIVER);
		return EINVAL;
	}

	/* Actually set the device's busid now. */
	DRM_LOCK();
	if (dev->unique_len || dev->unique) {
		DRM_UNLOCK();
		return EBUSY;
	}

	dev->unique_len = u->unique_len;
	dev->unique = busid;
	DRM_UNLOCK();

	return 0;
}


static int
drm_set_busid(struct drm_device *dev)
{

	DRM_LOCK();

	if (dev->unique != NULL) {
		DRM_UNLOCK();
		return EBUSY;
	}

	dev->unique_len = 20;
	dev->unique = malloc(dev->unique_len + 1, DRM_MEM_DRIVER, M_NOWAIT);
	if (dev->unique == NULL) {
		DRM_UNLOCK();
		return ENOMEM;
	}

	snprintf(dev->unique, dev->unique_len, "pci:%04x:%02x:%02x.%1x",
	    dev->pci_domain, dev->pci_bus, dev->pci_slot, dev->pci_func);

	DRM_UNLOCK();

	return 0;
}

int drm_getmap(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_map     *map = data;
	drm_local_map_t    *mapinlist;
	int          idx;
	int	     i = 0;

	idx = map->offset;

	DRM_LOCK();
	if (idx < 0) {
		DRM_UNLOCK();
		return EINVAL;
	}

	TAILQ_FOREACH(mapinlist, &dev->maplist, link) {
		if (i == idx) {
			map->offset = mapinlist->offset;
			map->size   = mapinlist->size;
			map->type   = mapinlist->type;
			map->flags  = mapinlist->flags;
			map->handle = mapinlist->handle;
			map->mtrr   = mapinlist->mtrr;
			break;
		}
		i++;
	}

	DRM_UNLOCK();

 	if (mapinlist == NULL)
		return EINVAL;

	return 0;
}

int drm_getclient(struct drm_device *dev, void *data,
		  struct drm_file *file_priv)
{
	struct drm_client *client = data;
	struct drm_file *pt;
	int idx;
	int i = 0;

	idx = client->idx;
	DRM_LOCK();
	TAILQ_FOREACH(pt, &dev->files, link) {
		if (i == idx) {
			client->auth  = pt->authenticated;
			client->pid   = pt->pid;
			client->uid   = pt->uid;
			client->magic = pt->magic;
			client->iocs  = pt->ioctl_count;
			DRM_UNLOCK();
			return 0;
		}
		i++;
	}
	DRM_UNLOCK();

	return EINVAL;
}

int drm_getstats(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_stats *stats = data;
	int          i;

	memset(stats, 0, sizeof(struct drm_stats));
	
	DRM_LOCK();

	for (i = 0; i < dev->counters; i++) {
		if (dev->types[i] == _DRM_STAT_LOCK)
			stats->data[i].value =
			    (dev->lock.hw_lock ? dev->lock.hw_lock->lock : 0);
		else 
			stats->data[i].value = atomic_read(&dev->counts[i]);
		stats->data[i].type = dev->types[i];
	}
	
	stats->count = dev->counters;

	DRM_UNLOCK();

	return 0;
}

#define DRM_IF_MAJOR	1
#define DRM_IF_MINOR	2

int drm_setversion(struct drm_device *dev, void *data,
		   struct drm_file *file_priv)
{
	struct drm_set_version *sv = data;
	struct drm_set_version ver;
	int if_version;

	/* Save the incoming data, and set the response before continuing
	 * any further.
	 */
	ver = *sv;
	sv->drm_di_major = DRM_IF_MAJOR;
	sv->drm_di_minor = DRM_IF_MINOR;
	sv->drm_dd_major = dev->driver->major;
	sv->drm_dd_minor = dev->driver->minor;

	if (ver.drm_di_major != -1) {
		if (ver.drm_di_major != DRM_IF_MAJOR ||
		    ver.drm_di_minor < 0 || ver.drm_di_minor > DRM_IF_MINOR) {
			return EINVAL;
		}
		if_version = DRM_IF_VERSION(ver.drm_di_major,
		    ver.drm_dd_minor);
		dev->if_version = DRM_MAX(if_version, dev->if_version);
		if (ver.drm_di_minor >= 1) {
			/*
			 * Version 1.1 includes tying of DRM to specific device
			 */
			drm_set_busid(dev);
		}
	}

	if (ver.drm_dd_major != -1) {
		if (ver.drm_dd_major != dev->driver->major ||
		    ver.drm_dd_minor < 0 ||
		    ver.drm_dd_minor > dev->driver->minor)
		{
			return EINVAL;
		}
	}

	return 0;
}


int drm_noop(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	DRM_DEBUG("\n");
	return 0;
}

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

/** @file drm_auth.c
 * Implementation of the get/authmagic ioctls implementing the authentication
 * scheme between the master and clients.
 */

#include "dev/drm/drmP.h"

static int drm_hash_magic(drm_magic_t magic)
{
	return magic & (DRM_HASH_SIZE-1);
}

/**
 * Returns the file private associated with the given magic number.
 */
static struct drm_file *drm_find_file(struct drm_device *dev, drm_magic_t magic)
{
	drm_magic_entry_t *pt;
	int hash = drm_hash_magic(magic);

	DRM_SPINLOCK_ASSERT(&dev->dev_lock);

	for (pt = dev->magiclist[hash].head; pt; pt = pt->next) {
		if (pt->magic == magic) {
			return pt->priv;
		}
	}

	return NULL;
}

/**
 * Inserts the given magic number into the hash table of used magic number
 * lists.
 */
static int drm_add_magic(struct drm_device *dev, struct drm_file *priv,
			 drm_magic_t magic)
{
	int		  hash;
	drm_magic_entry_t *entry;

	DRM_DEBUG("%d\n", magic);

	DRM_SPINLOCK_ASSERT(&dev->dev_lock);

	hash = drm_hash_magic(magic);
	entry = malloc(sizeof(*entry), DRM_MEM_MAGIC, M_ZERO | M_NOWAIT);
	if (!entry)
		return ENOMEM;
	entry->magic = magic;
	entry->priv  = priv;
	entry->next  = NULL;

	if (dev->magiclist[hash].tail) {
		dev->magiclist[hash].tail->next = entry;
		dev->magiclist[hash].tail	= entry;
	} else {
		dev->magiclist[hash].head	= entry;
		dev->magiclist[hash].tail	= entry;
	}

	return 0;
}

/**
 * Removes the given magic number from the hash table of used magic number
 * lists.
 */
static int drm_remove_magic(struct drm_device *dev, drm_magic_t magic)
{
	drm_magic_entry_t *prev = NULL;
	drm_magic_entry_t *pt;
	int		  hash;

	DRM_SPINLOCK_ASSERT(&dev->dev_lock);

	DRM_DEBUG("%d\n", magic);
	hash = drm_hash_magic(magic);

	for (pt = dev->magiclist[hash].head; pt; prev = pt, pt = pt->next) {
		if (pt->magic == magic) {
			if (dev->magiclist[hash].head == pt) {
				dev->magiclist[hash].head = pt->next;
			}
			if (dev->magiclist[hash].tail == pt) {
				dev->magiclist[hash].tail = prev;
			}
			if (prev) {
				prev->next = pt->next;
			}
			free(pt, DRM_MEM_MAGIC);
			return 0;
		}
	}

	return EINVAL;
}

/**
 * Called by the client, this returns a unique magic number to be authorized
 * by the master.
 *
 * The master may use its own knowledge of the client (such as the X
 * connection that the magic is passed over) to determine if the magic number
 * should be authenticated.
 */
int drm_getmagic(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	static drm_magic_t sequence = 0;
	struct drm_auth *auth = data;

	/* Find unique magic */
	if (file_priv->magic) {
		auth->magic = file_priv->magic;
	} else {
		DRM_LOCK();
		do {
			int old = sequence;

			auth->magic = old+1;

			if (!atomic_cmpset_int(&sequence, old, auth->magic))
				continue;
		} while (drm_find_file(dev, auth->magic));
		file_priv->magic = auth->magic;
		drm_add_magic(dev, file_priv, auth->magic);
		DRM_UNLOCK();
	}

	DRM_DEBUG("%u\n", auth->magic);

	return 0;
}

/**
 * Marks the client associated with the given magic number as authenticated.
 */
int drm_authmagic(struct drm_device *dev, void *data,
		  struct drm_file *file_priv)
{
	struct drm_auth *auth = data;
	struct drm_file *priv;

	DRM_DEBUG("%u\n", auth->magic);

	DRM_LOCK();
	priv = drm_find_file(dev, auth->magic);
	if (priv != NULL) {
		priv->authenticated = 1;
		drm_remove_magic(dev, auth->magic);
		DRM_UNLOCK();
		return 0;
	} else {
		DRM_UNLOCK();
		return EINVAL;
	}
}
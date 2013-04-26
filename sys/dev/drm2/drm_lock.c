
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

/** @file drm_lock.c
 * Implementation of the ioctls and other support code for dealing with the
 * hardware lock.
 *
 * The DRM hardware lock is a shared structure between the kernel and userland.
 *
 * On uncontended access where the new context was the last context, the
 * client may take the lock without dropping down into the kernel, using atomic
 * compare-and-set.
 *
 * If the client finds during compare-and-set that it was not the last owner
 * of the lock, it calls the DRM lock ioctl, which may sleep waiting for the
 * lock, and may have side-effects of kernel-managed context switching.
 *
 * When the client releases the lock, if the lock is marked as being contended
 * by another client, then the DRM unlock ioctl is called so that the
 * contending client may be woken up.
 */

#include <dev/drm2/drmP.h>

int drm_lock(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_lock *lock = data;
	int ret = 0;

	if (lock->context == DRM_KERNEL_CONTEXT) {
		DRM_ERROR("Process %d using kernel context %d\n",
		    DRM_CURRENTPID, lock->context);
		return EINVAL;
	}

	DRM_DEBUG("%d (pid %d) requests lock (0x%08x), flags = 0x%08x\n",
	    lock->context, DRM_CURRENTPID, dev->lock.hw_lock->lock,
	    lock->flags);

	if (drm_core_check_feature(dev, DRIVER_DMA_QUEUE) &&
	    lock->context < 0)
		return EINVAL;

	DRM_LOCK(dev);
	for (;;) {
		if (drm_lock_take(&dev->lock, lock->context)) {
			dev->lock.file_priv = file_priv;
			dev->lock.lock_time = jiffies;
			atomic_inc(&dev->counts[_DRM_STAT_LOCKS]);
			break;  /* Got lock */
		}

		/* Contention */
		ret = DRM_LOCK_SLEEP(dev, &dev->lock.lock_queue,
		    PCATCH, "drmlk2", 0);
		if (ret != 0)
			break;
	}
	DRM_UNLOCK(dev);

	if (ret == ERESTART)
		DRM_DEBUG("restarting syscall\n");
	else
		DRM_DEBUG("%d %s\n", lock->context,
		    ret ? "interrupted" : "has lock");

	if (ret != 0)
		return ret;

	/* XXX: Add signal blocking here */

	if (dev->driver->dma_quiescent != NULL &&
	    (lock->flags & _DRM_LOCK_QUIESCENT))
		dev->driver->dma_quiescent(dev);

	return 0;
}

int drm_unlock(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_lock *lock = data;

	DRM_DEBUG("%d (pid %d) requests unlock (0x%08x), flags = 0x%08x\n",
	    lock->context, DRM_CURRENTPID, dev->lock.hw_lock->lock,
	    lock->flags);

	if (lock->context == DRM_KERNEL_CONTEXT) {
		DRM_ERROR("Process %d using kernel context %d\n",
		    DRM_CURRENTPID, lock->context);
		return EINVAL;
	}

	atomic_inc(&dev->counts[_DRM_STAT_UNLOCKS]);

	DRM_LOCK(dev);
	drm_lock_transfer(&dev->lock, DRM_KERNEL_CONTEXT);

	if (drm_lock_free(&dev->lock, DRM_KERNEL_CONTEXT)) {
		DRM_ERROR("\n");
	}
	DRM_UNLOCK(dev);

	return 0;
}

int drm_lock_take(struct drm_lock_data *lock_data, unsigned int context)
{
	volatile unsigned int *lock = &lock_data->hw_lock->lock;
	unsigned int old, new;

	do {
		old = *lock;
		if (old & _DRM_LOCK_HELD)
			new = old | _DRM_LOCK_CONT;
		else
			new = context | _DRM_LOCK_HELD;
	} while (!atomic_cmpset_int(lock, old, new));

	if (_DRM_LOCKING_CONTEXT(old) == context) {
		if (old & _DRM_LOCK_HELD) {
			if (context != DRM_KERNEL_CONTEXT) {
				DRM_ERROR("%d holds heavyweight lock\n",
				    context);
			}
			return 0;
		}
	}
	if (new == (context | _DRM_LOCK_HELD)) {
		/* Have lock */
		return 1;
	}
	return 0;
}

/* This takes a lock forcibly and hands it to context.	Should ONLY be used
   inside *_unlock to give lock to kernel before calling *_dma_schedule. */
int drm_lock_transfer(struct drm_lock_data *lock_data, unsigned int context)
{
	volatile unsigned int *lock = &lock_data->hw_lock->lock;
	unsigned int old, new;

	lock_data->file_priv = NULL;
	do {
		old = *lock;
		new = context | _DRM_LOCK_HELD;
	} while (!atomic_cmpset_int(lock, old, new));

	return 1;
}

int drm_lock_free(struct drm_lock_data *lock_data, unsigned int context)
{
	volatile unsigned int *lock = &lock_data->hw_lock->lock;
	unsigned int old, new;

	lock_data->file_priv = NULL;
	do {
		old = *lock;
		new = 0;
	} while (!atomic_cmpset_int(lock, old, new));

	if (_DRM_LOCK_IS_HELD(old) && _DRM_LOCKING_CONTEXT(old) != context) {
		DRM_ERROR("%d freed heavyweight lock held by %d\n",
		    context, _DRM_LOCKING_CONTEXT(old));
		return 1;
	}
	DRM_WAKEUP_INT((void *)&lock_data->lock_queue);
	return 0;
}
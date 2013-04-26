
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

/** @file drm_fops.c
 * Support code for dealing with the file privates associated with each
 * open of the DRM device.
 */

#include <dev/drm2/drmP.h>

/* drm_open_helper is called whenever a process opens /dev/drm. */
int drm_open_helper(struct cdev *kdev, int flags, int fmt, DRM_STRUCTPROC *p,
		    struct drm_device *dev)
{
	struct drm_file *priv;
	int retcode;

	if (flags & O_EXCL)
		return EBUSY; /* No exclusive opens */
	dev->flags = flags;

	DRM_DEBUG("pid = %d, device = %s\n", DRM_CURRENTPID, devtoname(kdev));

	priv = malloc(sizeof(*priv), DRM_MEM_FILES, M_NOWAIT | M_ZERO);
	if (priv == NULL) {
		return ENOMEM;
	}

	DRM_LOCK(dev);
	priv->dev		= dev;
	priv->uid		= p->td_ucred->cr_svuid;
	priv->pid		= p->td_proc->p_pid;
	priv->ioctl_count 	= 0;

	/* for compatibility root is always authenticated */
	priv->authenticated	= DRM_SUSER(p);

	INIT_LIST_HEAD(&priv->fbs);
	INIT_LIST_HEAD(&priv->event_list);
	priv->event_space = 4096; /* set aside 4k for event buffer */

	if (dev->driver->driver_features & DRIVER_GEM)
		drm_gem_open(dev, priv);

	if (dev->driver->open) {
		/* shared code returns -errno */
		retcode = -dev->driver->open(dev, priv);
		if (retcode != 0) {
			free(priv, DRM_MEM_FILES);
			DRM_UNLOCK(dev);
			return retcode;
		}
	}

	/* first opener automatically becomes master */
	priv->master = TAILQ_EMPTY(&dev->files);

	TAILQ_INSERT_TAIL(&dev->files, priv, link);
	DRM_UNLOCK(dev);
	kdev->si_drv1 = dev;

	retcode = devfs_set_cdevpriv(priv, drm_close);
	if (retcode != 0)
		drm_close(priv);

	return (retcode);
}

static bool
drm_dequeue_event(struct drm_device *dev, struct drm_file *file_priv,
    struct uio *uio, struct drm_pending_event **out)
{
	struct drm_pending_event *e;

	if (list_empty(&file_priv->event_list))
		return (false);
	e = list_first_entry(&file_priv->event_list,
	    struct drm_pending_event, link);
	if (e->event->length > uio->uio_resid)
		return (false);

	file_priv->event_space += e->event->length;
	list_del(&e->link);
	*out = e;
	return (true);
}

int
drm_read(struct cdev *kdev, struct uio *uio, int ioflag)
{
	struct drm_file *file_priv;
	struct drm_device *dev;
	struct drm_pending_event *e;
	int error;

	error = devfs_get_cdevpriv((void **)&file_priv);
	if (error != 0) {
		DRM_ERROR("can't find authenticator\n");
		return (EINVAL);
	}
	dev = drm_get_device_from_kdev(kdev);
	mtx_lock(&dev->event_lock);
	while (list_empty(&file_priv->event_list)) {
		if ((ioflag & O_NONBLOCK) != 0) {
			error = EAGAIN;
			goto out;
		}
		error = msleep(&file_priv->event_space, &dev->event_lock,
	           PCATCH, "drmrea", 0);
	       if (error != 0)
		       goto out;
	}
	while (drm_dequeue_event(dev, file_priv, uio, &e)) {
		mtx_unlock(&dev->event_lock);
		error = uiomove(e->event, e->event->length, uio);
		CTR3(KTR_DRM, "drm_event_dequeued %d %d %d", curproc->p_pid,
		    e->event->type, e->event->length);
		e->destroy(e);
		if (error != 0)
			return (error);
		mtx_lock(&dev->event_lock);
	}
out:
	mtx_unlock(&dev->event_lock);
	return (error);
}

void
drm_event_wakeup(struct drm_pending_event *e)
{
	struct drm_file *file_priv;
	struct drm_device *dev;

	file_priv = e->file_priv;
	dev = file_priv->dev;
	mtx_assert(&dev->event_lock, MA_OWNED);

	wakeup(&file_priv->event_space);
	selwakeup(&file_priv->event_poll);
}

int
drm_poll(struct cdev *kdev, int events, struct thread *td)
{
	struct drm_file *file_priv;
	struct drm_device *dev;
	int error, revents;

	error = devfs_get_cdevpriv((void **)&file_priv);
	if (error != 0) {
		DRM_ERROR("can't find authenticator\n");
		return (EINVAL);
	}
	dev = drm_get_device_from_kdev(kdev);

	revents = 0;
	mtx_lock(&dev->event_lock);
	if ((events & (POLLIN | POLLRDNORM)) != 0) {
		if (list_empty(&file_priv->event_list)) {
			CTR0(KTR_DRM, "drm_poll empty list");
			selrecord(td, &file_priv->event_poll);
		} else {
			revents |= events & (POLLIN | POLLRDNORM);
			CTR1(KTR_DRM, "drm_poll revents %x", revents);
		}
	}
	mtx_unlock(&dev->event_lock);
	return (revents);
}
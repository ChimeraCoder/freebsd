
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

#include "dev/drm/drmP.h"

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

	DRM_LOCK();
	priv->dev		= dev;
	priv->uid		= p->td_ucred->cr_svuid;
	priv->pid		= p->td_proc->p_pid;
	priv->ioctl_count 	= 0;

	/* for compatibility root is always authenticated */
	priv->authenticated	= DRM_SUSER(p);

	if (dev->driver->open) {
		/* shared code returns -errno */
		retcode = -dev->driver->open(dev, priv);
		if (retcode != 0) {
			free(priv, DRM_MEM_FILES);
			DRM_UNLOCK();
			return retcode;
		}
	}

	/* first opener automatically becomes master */
	priv->master = TAILQ_EMPTY(&dev->files);

	TAILQ_INSERT_TAIL(&dev->files, priv, link);
	DRM_UNLOCK();
	kdev->si_drv1 = dev;

	retcode = devfs_set_cdevpriv(priv, drm_close);
	if (retcode != 0)
		drm_close(priv);

	return (retcode);
}


/* The drm_read and drm_poll are stubs to prevent spurious errors
 * on older X Servers (4.3.0 and earlier) */

int drm_read(struct cdev *kdev, struct uio *uio, int ioflag)
{
	return 0;
}

int drm_poll(struct cdev *kdev, int events, DRM_STRUCTPROC *p)
{
	return 0;
}
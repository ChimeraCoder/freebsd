
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

#include "dev/drm/drmP.h"
#include "dev/drm/drm.h"
#include "dev/drm/mach64_drm.h"
#include "dev/drm/mach64_drv.h"

irqreturn_t mach64_driver_irq_handler(DRM_IRQ_ARGS)
{
	struct drm_device *dev = arg;
	drm_mach64_private_t *dev_priv = dev->dev_private;
	int status;

	status = MACH64_READ(MACH64_CRTC_INT_CNTL);

	/* VBLANK interrupt */
	if (status & MACH64_CRTC_VBLANK_INT) {
		/* Mask off all interrupt ack bits before setting the ack bit, since
		 * there may be other handlers outside the DRM.
		 *
		 * NOTE: On mach64, you need to keep the enable bits set when doing
		 * the ack, despite what the docs say about not acking and enabling
		 * in a single write.
		 */
		MACH64_WRITE(MACH64_CRTC_INT_CNTL,
			     (status & ~MACH64_CRTC_INT_ACKS)
			     | MACH64_CRTC_VBLANK_INT);

		atomic_inc(&dev_priv->vbl_received);
		drm_handle_vblank(dev, 0);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

u32 mach64_get_vblank_counter(struct drm_device * dev, int crtc)
{
	const drm_mach64_private_t *const dev_priv = dev->dev_private;

	if (crtc != 0)
		return 0;

	return atomic_read(&dev_priv->vbl_received);
}

int mach64_enable_vblank(struct drm_device * dev, int crtc)
{
	drm_mach64_private_t *dev_priv = dev->dev_private;
	u32 status = MACH64_READ(MACH64_CRTC_INT_CNTL);

	if (crtc != 0) {
		DRM_ERROR("tried to enable vblank on non-existent crtc %d\n",
			  crtc);
		return -EINVAL;
	}

	DRM_DEBUG("before enable vblank CRTC_INT_CTNL: 0x%08x\n", status);

	/* Turn on VBLANK interrupt */
	MACH64_WRITE(MACH64_CRTC_INT_CNTL, MACH64_READ(MACH64_CRTC_INT_CNTL)
		     | MACH64_CRTC_VBLANK_INT_EN);

	return 0;
}

void mach64_disable_vblank(struct drm_device * dev, int crtc)
{
	if (crtc != 0) {
		DRM_ERROR("tried to disable vblank on non-existent crtc %d\n",
			  crtc);
		return;
	}

	/*
	 * FIXME: implement proper interrupt disable by using the vblank
	 * counter register (if available).
	 */
}

static void mach64_disable_vblank_local(struct drm_device * dev, int crtc)
{
	drm_mach64_private_t *dev_priv = dev->dev_private;
	u32 status = MACH64_READ(MACH64_CRTC_INT_CNTL);

	if (crtc != 0) {
		DRM_ERROR("tried to disable vblank on non-existent crtc %d\n",
			  crtc);
		return;
	}

	DRM_DEBUG("before disable vblank CRTC_INT_CTNL: 0x%08x\n", status);

	/* Disable and clear VBLANK interrupt */
	MACH64_WRITE(MACH64_CRTC_INT_CNTL, (status & ~MACH64_CRTC_VBLANK_INT_EN)
		     | MACH64_CRTC_VBLANK_INT);
}

void mach64_driver_irq_preinstall(struct drm_device * dev)
{
	drm_mach64_private_t *dev_priv = dev->dev_private;

	u32 status = MACH64_READ(MACH64_CRTC_INT_CNTL);

	DRM_DEBUG("before install CRTC_INT_CTNL: 0x%08x\n", status);

	mach64_disable_vblank_local(dev, 0);
}

int mach64_driver_irq_postinstall(struct drm_device * dev)
{
	return 0;
}

void mach64_driver_irq_uninstall(struct drm_device * dev)
{
	drm_mach64_private_t *dev_priv = dev->dev_private;
	if (!dev_priv)
		return;

	mach64_disable_vblank_local(dev, 0);

	DRM_DEBUG("after uninstall CRTC_INT_CTNL: 0x%08x\n",
		  MACH64_READ(MACH64_CRTC_INT_CNTL));
}
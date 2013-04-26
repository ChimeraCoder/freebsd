
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
#include "dev/drm/r128_drm.h"
#include "dev/drm/r128_drv.h"
#include "dev/drm/drm_pciids.h"

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t r128_pciidlist[] = {
	r128_PCI_IDS
};

static void r128_configure(struct drm_device *dev)
{
	dev->driver->driver_features =
	    DRIVER_USE_AGP | DRIVER_USE_MTRR | DRIVER_PCI_DMA |
	    DRIVER_SG | DRIVER_HAVE_DMA | DRIVER_HAVE_IRQ;

	dev->driver->buf_priv_size	= sizeof(drm_r128_buf_priv_t);
	dev->driver->load		= r128_driver_load;
	dev->driver->preclose		= r128_driver_preclose;
	dev->driver->lastclose		= r128_driver_lastclose;
	dev->driver->get_vblank_counter	= r128_get_vblank_counter;
	dev->driver->enable_vblank	= r128_enable_vblank;
	dev->driver->disable_vblank	= r128_disable_vblank;
	dev->driver->irq_preinstall	= r128_driver_irq_preinstall;
	dev->driver->irq_postinstall	= r128_driver_irq_postinstall;
	dev->driver->irq_uninstall	= r128_driver_irq_uninstall;
	dev->driver->irq_handler	= r128_driver_irq_handler;
	dev->driver->dma_ioctl		= r128_cce_buffers;

	dev->driver->ioctls		= r128_ioctls;
	dev->driver->max_ioctl		= r128_max_ioctl;

	dev->driver->name		= DRIVER_NAME;
	dev->driver->desc		= DRIVER_DESC;
	dev->driver->date		= DRIVER_DATE;
	dev->driver->major		= DRIVER_MAJOR;
	dev->driver->minor		= DRIVER_MINOR;
	dev->driver->patchlevel		= DRIVER_PATCHLEVEL;
}

static int
r128_probe(device_t kdev)
{
	return drm_probe(kdev, r128_pciidlist);
}

static int
r128_attach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);

	dev->driver = malloc(sizeof(struct drm_driver_info), DRM_MEM_DRIVER,
	    M_WAITOK | M_ZERO);

	r128_configure(dev);

	return drm_attach(kdev, r128_pciidlist);
}

int r128_driver_load(struct drm_device * dev, unsigned long flags)
{
	return drm_vblank_init(dev, 1);
}

static int
r128_detach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);
	int ret;

	ret = drm_detach(kdev);

	free(dev->driver, DRM_MEM_DRIVER);

	return ret;
}

static device_method_t r128_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		r128_probe),
	DEVMETHOD(device_attach,	r128_attach),
	DEVMETHOD(device_detach,	r128_detach),

	{ 0, 0 }
};

static driver_t r128_driver = {
	"drm",
	r128_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
#if __FreeBSD_version >= 700010
DRIVER_MODULE(r128, vgapci, r128_driver, drm_devclass, 0, 0);
#else
DRIVER_MODULE(r128, pci, r128_driver, drm_devclass, 0, 0);
#endif
MODULE_DEPEND(r128, drm, 1, 1, 1);
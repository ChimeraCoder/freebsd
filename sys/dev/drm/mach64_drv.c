
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


#include <sys/types.h>

#include "dev/drm/drmP.h"
#include "dev/drm/drm.h"
#include "dev/drm/mach64_drm.h"
#include "dev/drm/mach64_drv.h"
#include "dev/drm/drm_pciids.h"

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t mach64_pciidlist[] = {
	mach64_PCI_IDS
};

static void mach64_configure(struct drm_device *dev)
{
	dev->driver->driver_features =
	    DRIVER_USE_AGP | DRIVER_USE_MTRR | DRIVER_PCI_DMA |
	    DRIVER_HAVE_DMA | DRIVER_HAVE_IRQ;

	dev->driver->buf_priv_size	= 1; /* No dev_priv */
	dev->driver->load		= mach64_driver_load;
	dev->driver->lastclose		= mach64_driver_lastclose;
	dev->driver->get_vblank_counter	= mach64_get_vblank_counter;
	dev->driver->enable_vblank	= mach64_enable_vblank;
	dev->driver->disable_vblank	= mach64_disable_vblank;
	dev->driver->irq_preinstall	= mach64_driver_irq_preinstall;
	dev->driver->irq_postinstall	= mach64_driver_irq_postinstall;
	dev->driver->irq_uninstall	= mach64_driver_irq_uninstall;
	dev->driver->irq_handler	= mach64_driver_irq_handler;
	dev->driver->dma_ioctl		= mach64_dma_buffers;

	dev->driver->ioctls		= mach64_ioctls;
	dev->driver->max_ioctl		= mach64_max_ioctl;

	dev->driver->name		= DRIVER_NAME;
	dev->driver->desc		= DRIVER_DESC;
	dev->driver->date		= DRIVER_DATE;
	dev->driver->major		= DRIVER_MAJOR;
	dev->driver->minor		= DRIVER_MINOR;
	dev->driver->patchlevel		= DRIVER_PATCHLEVEL;
}

static int
mach64_probe(device_t kdev)
{
	return drm_probe(kdev, mach64_pciidlist);
}

static int
mach64_attach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);

	dev->driver = malloc(sizeof(struct drm_driver_info), DRM_MEM_DRIVER,
	    M_WAITOK | M_ZERO);

	mach64_configure(dev);

	return drm_attach(kdev, mach64_pciidlist);
}

int
mach64_driver_load(struct drm_device * dev, unsigned long flags)
{
        return drm_vblank_init(dev, 1);
}

static int
mach64_detach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);
	int ret;

	ret = drm_detach(kdev);

	free(dev->driver, DRM_MEM_DRIVER);

	return ret;
}

static device_method_t mach64_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		mach64_probe),
	DEVMETHOD(device_attach,	mach64_attach),
	DEVMETHOD(device_detach,	mach64_detach),

	{ 0, 0 }
};

static driver_t mach64_driver = {
	"drm",
	mach64_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
#if __FreeBSD_version >= 700010
DRIVER_MODULE(mach64, vgapci, mach64_driver, drm_devclass, 0, 0);
#else
DRIVER_MODULE(mach64, pci, mach64_driver, drm_devclass, 0, 0);
#endif
MODULE_DEPEND(mach64, drm, 1, 1, 1);
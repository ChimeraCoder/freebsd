
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
#include "dev/drm/radeon_drm.h"
#include "dev/drm/radeon_drv.h"
#include "dev/drm/drm_pciids.h"

int radeon_no_wb;

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t radeon_pciidlist[] = {
	radeon_PCI_IDS
};

static void radeon_configure(struct drm_device *dev)
{
	dev->driver->driver_features =
	    DRIVER_USE_AGP | DRIVER_USE_MTRR | DRIVER_PCI_DMA |
	    DRIVER_SG | DRIVER_HAVE_DMA | DRIVER_HAVE_IRQ;

	dev->driver->buf_priv_size	= sizeof(drm_radeon_buf_priv_t);
	dev->driver->load		= radeon_driver_load;
	dev->driver->unload		= radeon_driver_unload;
	dev->driver->firstopen		= radeon_driver_firstopen;
	dev->driver->open		= radeon_driver_open;
	dev->driver->preclose		= radeon_driver_preclose;
	dev->driver->postclose		= radeon_driver_postclose;
	dev->driver->lastclose		= radeon_driver_lastclose;
	dev->driver->get_vblank_counter	= radeon_get_vblank_counter;
	dev->driver->enable_vblank	= radeon_enable_vblank;
	dev->driver->disable_vblank	= radeon_disable_vblank;
	dev->driver->irq_preinstall	= radeon_driver_irq_preinstall;
	dev->driver->irq_postinstall	= radeon_driver_irq_postinstall;
	dev->driver->irq_uninstall	= radeon_driver_irq_uninstall;
	dev->driver->irq_handler	= radeon_driver_irq_handler;
	dev->driver->dma_ioctl		= radeon_cp_buffers;

	dev->driver->ioctls		= radeon_ioctls;
	dev->driver->max_ioctl		= radeon_max_ioctl;

	dev->driver->name		= DRIVER_NAME;
	dev->driver->desc		= DRIVER_DESC;
	dev->driver->date		= DRIVER_DATE;
	dev->driver->major		= DRIVER_MAJOR;
	dev->driver->minor		= DRIVER_MINOR;
	dev->driver->patchlevel		= DRIVER_PATCHLEVEL;
}

static int
radeon_probe(device_t kdev)
{
	return drm_probe(kdev, radeon_pciidlist);
}

static int
radeon_attach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);

	dev->driver = malloc(sizeof(struct drm_driver_info), DRM_MEM_DRIVER,
	    M_WAITOK | M_ZERO);

	radeon_configure(dev);

	return drm_attach(kdev, radeon_pciidlist);
}

static int
radeon_detach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);
	int ret;

	ret = drm_detach(kdev);

	free(dev->driver, DRM_MEM_DRIVER);

	return ret;
}

static device_method_t radeon_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		radeon_probe),
	DEVMETHOD(device_attach,	radeon_attach),
	DEVMETHOD(device_detach,	radeon_detach),

	{ 0, 0 }
};

static driver_t radeon_driver = {
	"drm",
	radeon_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
#if __FreeBSD_version >= 700010
DRIVER_MODULE(radeon, vgapci, radeon_driver, drm_devclass, 0, 0);
#else
DRIVER_MODULE(radeon, pci, radeon_driver, drm_devclass, 0, 0);
#endif
MODULE_DEPEND(radeon, drm, 1, 1, 1);

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
#include "dev/drm/via_drm.h"
#include "dev/drm/via_drv.h"
#include "dev/drm/drm_pciids.h"

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t via_pciidlist[] = {
	viadrv_PCI_IDS
};

static void via_configure(struct drm_device *dev)
{
	dev->driver->driver_features =
	    DRIVER_USE_AGP | DRIVER_USE_MTRR | DRIVER_HAVE_IRQ;

	dev->driver->buf_priv_size	= sizeof(drm_via_private_t);
	dev->driver->load		= via_driver_load;
	dev->driver->unload		= via_driver_unload;
	dev->driver->lastclose		= via_lastclose;
	dev->driver->get_vblank_counter	= via_get_vblank_counter;
	dev->driver->enable_vblank	= via_enable_vblank;
	dev->driver->disable_vblank	= via_disable_vblank;
	dev->driver->irq_preinstall	= via_driver_irq_preinstall;
	dev->driver->irq_postinstall	= via_driver_irq_postinstall;
	dev->driver->irq_uninstall	= via_driver_irq_uninstall;
	dev->driver->irq_handler	= via_driver_irq_handler;
	dev->driver->dma_quiescent	= via_driver_dma_quiescent;

	dev->driver->ioctls		= via_ioctls;
	dev->driver->max_ioctl		= via_max_ioctl;

	dev->driver->name		= DRIVER_NAME;
	dev->driver->desc		= DRIVER_DESC;
	dev->driver->date		= DRIVER_DATE;
	dev->driver->major		= DRIVER_MAJOR;
	dev->driver->minor		= DRIVER_MINOR;
	dev->driver->patchlevel		= DRIVER_PATCHLEVEL;
}

static int
via_probe(device_t kdev)
{
	return drm_probe(kdev, via_pciidlist);
}

static int
via_attach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);

	dev->driver = malloc(sizeof(struct drm_driver_info), DRM_MEM_DRIVER,
	    M_WAITOK | M_ZERO);

	via_configure(dev);

	return drm_attach(kdev, via_pciidlist);
}

static int
via_detach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);
	int ret;

	ret = drm_detach(kdev);

	free(dev->driver, DRM_MEM_DRIVER);

	return ret;
}

static device_method_t via_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		via_probe),
	DEVMETHOD(device_attach,	via_attach),
	DEVMETHOD(device_detach,	via_detach),

	{ 0, 0 }
};

static driver_t via_driver = {
	"drm",
	via_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
DRIVER_MODULE(via, vgapci, via_driver, drm_devclass, 0, 0);
MODULE_DEPEND(via, drm, 1, 1, 1);
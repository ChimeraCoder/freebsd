
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
#include "dev/drm/sis_drm.h"
#include "dev/drm/sis_drv.h"
#include "dev/drm/drm_pciids.h"

/* drv_PCI_IDs comes from drm_pciids.h, generated from drm_pciids.txt. */
static drm_pci_id_list_t sis_pciidlist[] = {
	sis_PCI_IDS
};

static void sis_configure(struct drm_device *dev)
{
	dev->driver->driver_features =
	    DRIVER_USE_AGP | DRIVER_USE_MTRR;

	dev->driver->buf_priv_size	= 1; /* No dev_priv */
	dev->driver->context_ctor	= sis_init_context;
	dev->driver->context_dtor	= sis_final_context;

	dev->driver->ioctls		= sis_ioctls;
	dev->driver->max_ioctl		= sis_max_ioctl;

	dev->driver->name		= DRIVER_NAME;
	dev->driver->desc		= DRIVER_DESC;
	dev->driver->date		= DRIVER_DATE;
	dev->driver->major		= DRIVER_MAJOR;
	dev->driver->minor		= DRIVER_MINOR;
	dev->driver->patchlevel		= DRIVER_PATCHLEVEL;
}

static int
sis_probe(device_t kdev)
{
	return drm_probe(kdev, sis_pciidlist);
}

static int
sis_attach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);

	dev->driver = malloc(sizeof(struct drm_driver_info), DRM_MEM_DRIVER,
	    M_WAITOK | M_ZERO);

	sis_configure(dev);

	return drm_attach(kdev, sis_pciidlist);
}

static int
sis_detach(device_t kdev)
{
	struct drm_device *dev = device_get_softc(kdev);
	int ret;

	ret = drm_detach(kdev);

	free(dev->driver, DRM_MEM_DRIVER);

	return ret;
}

static device_method_t sis_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		sis_probe),
	DEVMETHOD(device_attach,	sis_attach),
	DEVMETHOD(device_detach,	sis_detach),

	{ 0, 0 }
};

static driver_t sis_driver = {
	"drm",
	sis_methods,
	sizeof(struct drm_device)
};

extern devclass_t drm_devclass;
#if __FreeBSD_version >= 700010
DRIVER_MODULE(sisdrm, vgapci, sis_driver, drm_devclass, 0, 0);
#else
DRIVER_MODULE(sisdrm, pci, sis_driver, drm_devclass, 0, 0);
#endif
MODULE_DEPEND(sisdrm, drm, 1, 1, 1);
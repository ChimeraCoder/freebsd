
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

#include <sys/stdint.h>
#include <sys/stddef.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/condvar.h>
#include <sys/sysctl.h>
#include <sys/sx.h>
#include <sys/unistd.h>
#include <sys/callout.h>
#include <sys/malloc.h>
#include <sys/priv.h>

#include <sys/rman.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbdi.h>

#include <dev/usb/usb_core.h>
#include <dev/usb/usb_busdma.h>
#include <dev/usb/usb_process.h>
#include <dev/usb/usb_util.h>

#include <dev/usb/usb_controller.h>
#include <dev/usb/usb_bus.h>
#include <dev/usb/controller/ehci.h>
#include <dev/usb/controller/ehcireg.h>

#include "ps3bus.h"

struct ps3_ehci_softc {
	ehci_softc_t            base;
	struct bus_space         tag;
};

static int
ehci_ps3_probe(device_t dev)
{
	if (ps3bus_get_bustype(dev) != PS3_BUSTYPE_SYSBUS ||
	    ps3bus_get_devtype(dev) != PS3_DEVTYPE_USB)
		return (ENXIO);

	device_set_desc(dev, "Playstation 3 USB 2.0 controller");
	return (BUS_PROBE_SPECIFIC);
}

static int
ehci_ps3_attach(device_t dev)
{
	ehci_softc_t *sc = device_get_softc(dev);
	int rid, err;

	sc->sc_bus.parent = dev;
	sc->sc_bus.devices = sc->sc_devices;
	sc->sc_bus.devices_max = EHCI_MAX_DEVICES;

	if (usb_bus_mem_alloc_all(&sc->sc_bus,
	    USB_GET_DMA_TAG(dev), &ehci_iterate_hw_softc))
		return (ENOMEM);

	rid = 1;
	sc->sc_io_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &rid, RF_ACTIVE);

	if (!sc->sc_io_res) {
		device_printf(dev, "Could not map memory\n");
		goto error;
	}

	sc->sc_io_tag = rman_get_bustag(sc->sc_io_res);
	sc->sc_io_hdl = rman_get_bushandle(sc->sc_io_res);
	sc->sc_io_size = rman_get_size(sc->sc_io_res);

	rid = 1;
	sc->sc_irq_res = bus_alloc_resource_any(dev, SYS_RES_IRQ, &rid,
	    RF_SHAREABLE | RF_ACTIVE);

	if (sc->sc_irq_res == NULL) {
		device_printf(dev, "Could not allocate irq\n");
		return (ENXIO);
	}

	sc->sc_bus.bdev = device_add_child(dev, "usbus", -1);
	if (!sc->sc_bus.bdev) {
		device_printf(dev, "Could not add USB device\n");
		return (ENXIO);
	}

	device_set_ivars(sc->sc_bus.bdev, &sc->sc_bus);

	sprintf(sc->sc_vendor, "Sony");

	err = bus_setup_intr(dev, sc->sc_irq_res, INTR_TYPE_BIO | INTR_MPSAFE,
	    NULL, (driver_intr_t *)ehci_interrupt, sc, &sc->sc_intr_hdl);
	if (err) {
		device_printf(dev, "Could not setup error irq, %d\n", err);
		goto error;
	}

	sc->sc_flags |= EHCI_SCFLG_BIGEMMIO;
	err = ehci_init(sc);
	if (err) {
		device_printf(dev, "USB init failed err=%d\n", err);
		goto error;
	}

	err = device_probe_and_attach(sc->sc_bus.bdev);
	if (err == 0)
		return (0);

error:
	return (ENXIO);
}

static device_method_t ehci_ps3_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, ehci_ps3_probe),
	DEVMETHOD(device_attach, ehci_ps3_attach),
	DEVMETHOD(device_resume, bus_generic_resume),
	DEVMETHOD(device_suspend, bus_generic_suspend),
	DEVMETHOD(device_shutdown, bus_generic_shutdown),

	DEVMETHOD_END
};

static driver_t ehci_ps3_driver = {
	.name = "ehci",
	.methods = ehci_ps3_methods,
	.size = sizeof(ehci_softc_t),
};

static devclass_t ehci_ps3_devclass;

DRIVER_MODULE(ehci_ps3, ps3bus, ehci_ps3_driver, ehci_ps3_devclass, 0, 0);
MODULE_DEPEND(ehci_ps3, usb, 1, 1, 1);
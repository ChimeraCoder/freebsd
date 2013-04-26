
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

#include <dev/usb/controller/dotg.h>
#include <mips/rt305x/rt305xreg.h>
#include <mips/rt305x/rt305x_sysctlvar.h>

#define	MEM_RID	0

static device_probe_t dotg_obio_probe;
static device_attach_t dotg_obio_attach;
static device_detach_t dotg_obio_detach;

struct dotg_obio_softc {
	struct dotg_softc sc_dci;	/* must be first */
};

static int
dotg_obio_probe(device_t dev)
{
	device_set_desc(dev, "DWC like USB OTG controller");
	return (0);
}

static int
dotg_obio_attach(device_t dev)
{
	struct dotg_obio_softc *sc = device_get_softc(dev);
	int err;

	/* setup controller interface softc */

	/* initialise some bus fields */
	sc->sc_dci.sc_dev = dev;
	sc->sc_dci.sc_bus.parent = dev;
	sc->sc_dci.sc_bus.devices = sc->sc_dci.sc_devices;
	sc->sc_dci.sc_bus.devices_max = DOTG_MAX_DEVICES;

	/* get all DMA memory */
	if (usb_bus_mem_alloc_all(&sc->sc_dci.sc_bus,
	    USB_GET_DMA_TAG(dev), NULL)) {
		printf("No mem\n");
		return (ENOMEM);
	}
	sc->sc_dci.sc_mem_rid = 0;
	sc->sc_dci.sc_mem_res =
	    bus_alloc_resource_any(dev, SYS_RES_MEMORY, &sc->sc_dci.sc_irq_rid,
			       RF_ACTIVE);
	if (!(sc->sc_dci.sc_mem_res)) {
		printf("Can`t alloc MEM\n");
		goto error;
	}
	sc->sc_dci.sc_bst = rman_get_bustag(sc->sc_dci.sc_mem_res);
	sc->sc_dci.sc_bsh = rman_get_bushandle(sc->sc_dci.sc_mem_res);

	sc->sc_dci.sc_irq_rid = 0;
	sc->sc_dci.sc_irq_res = bus_alloc_resource_any(dev, SYS_RES_IRQ, 
	    &sc->sc_dci.sc_irq_rid, RF_SHAREABLE| RF_ACTIVE);
	if (!(sc->sc_dci.sc_irq_res)) {
		printf("Can`t alloc IRQ\n");
		goto error;
	}

	sc->sc_dci.sc_bus.bdev = device_add_child(dev, "usbus", -1);
	if (!(sc->sc_dci.sc_bus.bdev)) {
		printf("Can`t add usbus\n");
		goto error;
	}
	device_set_ivars(sc->sc_dci.sc_bus.bdev, &sc->sc_dci.sc_bus);

#if (__FreeBSD_version >= 700031)
	err = bus_setup_intr(dev, sc->sc_dci.sc_irq_res,
	    INTR_TYPE_BIO | INTR_MPSAFE, NULL, (driver_intr_t *)dotg_interrupt,
	    sc, &sc->sc_dci.sc_intr_hdl);
#else
	err = bus_setup_intr(dev, sc->sc_dci.sc_irq_res,
	    INTR_TYPE_BIO | INTR_MPSAFE, (driver_intr_t *)dotg_interrupt,
	    sc, &sc->sc_dci.sc_intr_hdl);
#endif
	if (err) {
		sc->sc_dci.sc_intr_hdl = NULL;
		printf("Can`t set IRQ handle\n");
		goto error;
	}

	/* Run clock for OTG core */
	rt305x_sysctl_set(SYSCTL_CLKCFG1, rt305x_sysctl_get(SYSCTL_CLKCFG1) | 
	    SYSCTL_CLKCFG1_OTG_CLK_EN);
	rt305x_sysctl_set(SYSCTL_RSTCTRL, SYSCTL_RSTCTRL_OTG);
	DELAY(100);

	err = dotg_init(&sc->sc_dci);
	if (err) printf("dotg_init fail\n");
	if (!err) {
		err = device_probe_and_attach(sc->sc_dci.sc_bus.bdev);
		if (err) printf("device_probe_and_attach fail\n");
	}
	if (err) {
		goto error;
	}
	return (0);

error:
	dotg_obio_detach(dev);
	return (ENXIO);
}

static int
dotg_obio_detach(device_t dev)
{
	struct dotg_obio_softc *sc = device_get_softc(dev);
	device_t bdev;
	int err;

	if (sc->sc_dci.sc_bus.bdev) {
		bdev = sc->sc_dci.sc_bus.bdev;
		device_detach(bdev);
		device_delete_child(dev, bdev);
	}
	/* during module unload there are lots of children leftover */
	device_delete_children(dev);

	if (sc->sc_dci.sc_irq_res && sc->sc_dci.sc_intr_hdl) {
		/*
		 * only call dotg_obio_uninit() after dotg_obio_init()
		 */
		dotg_uninit(&sc->sc_dci);

		/* Stop OTG clock */
		rt305x_sysctl_set(SYSCTL_CLKCFG1, 
		    rt305x_sysctl_get(SYSCTL_CLKCFG1) & 
		    ~SYSCTL_CLKCFG1_OTG_CLK_EN);

		err = bus_teardown_intr(dev, sc->sc_dci.sc_irq_res,
		    sc->sc_dci.sc_intr_hdl);
		sc->sc_dci.sc_intr_hdl = NULL;
	}
	if (sc->sc_dci.sc_irq_res) {
		bus_release_resource(dev, SYS_RES_IRQ, 0,
		    sc->sc_dci.sc_irq_res);
		sc->sc_dci.sc_irq_res = NULL;
	}
	if (sc->sc_dci.sc_mem_res) {
		bus_release_resource(dev, SYS_RES_MEMORY, 0,
		    sc->sc_dci.sc_mem_res);
		sc->sc_dci.sc_mem_res = NULL;
	}
	usb_bus_mem_free_all(&sc->sc_dci.sc_bus, NULL);

	return (0);
}

static device_method_t dotg_obio_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, dotg_obio_probe),
	DEVMETHOD(device_attach, dotg_obio_attach),
	DEVMETHOD(device_detach, dotg_obio_detach),
	DEVMETHOD(device_suspend, bus_generic_suspend),
	DEVMETHOD(device_resume, bus_generic_resume),
	DEVMETHOD(device_shutdown, bus_generic_shutdown),

	DEVMETHOD_END
};

static driver_t dotg_obio_driver = {
	.name = "dotg",
	.methods = dotg_obio_methods,
	.size = sizeof(struct dotg_obio_softc),
};

static devclass_t dotg_obio_devclass;

DRIVER_MODULE(dotg, obio, dotg_obio_driver, dotg_obio_devclass, 0, 0);
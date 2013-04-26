
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

/*
 * Device specific driver for the SUNW,envctrl device found on some
 * UltraSPARC Sun systems.  This device is a Philips PCF8584 sitting
 * on the Ebus2.
 */

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/resource.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <sys/rman.h>

#include <dev/iicbus/iicbus.h>
#include <dev/iicbus/iiconf.h>
#include <dev/pcf/pcfvar.h>
#include "iicbus_if.h"

#undef PCF_DEFAULT_ADDR
#define PCF_DEFAULT_ADDR	0x55 /* SUNW,pcf default */

static int envctrl_probe(device_t);
static int envctrl_attach(device_t);
static int envctrl_detach(device_t);

static device_method_t envctrl_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		envctrl_probe),
	DEVMETHOD(device_attach,	envctrl_attach),
	DEVMETHOD(device_detach,	envctrl_detach),

	/* iicbus interface */
	DEVMETHOD(iicbus_callback,	iicbus_null_callback),
	DEVMETHOD(iicbus_repeated_start, pcf_repeated_start),
	DEVMETHOD(iicbus_start,		pcf_start),
	DEVMETHOD(iicbus_stop,		pcf_stop),
	DEVMETHOD(iicbus_write,		pcf_write),
	DEVMETHOD(iicbus_read,		pcf_read),
	DEVMETHOD(iicbus_reset,		pcf_rst_card),
	{ 0, 0 }
};

static devclass_t envctrl_devclass;

static driver_t envctrl_driver = {
	"envctrl",
	envctrl_methods,
	sizeof(struct pcf_softc),
};

static int
envctrl_probe(device_t dev)
{

	if (strcmp("SUNW,envctrl", ofw_bus_get_name(dev)) == 0) {
		device_set_desc(dev, "EBus SUNW,envctrl");
		return (0);
	}
	return (ENXIO);
}

static int
envctrl_attach(device_t dev)
{
	struct pcf_softc *sc;
	int rv = ENXIO;

	sc = DEVTOSOFTC(dev);
	mtx_init(&sc->pcf_lock, device_get_nameunit(dev), "pcf", MTX_DEF);

	/* IO port is mandatory */
	sc->res_ioport = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
						&sc->rid_ioport, RF_ACTIVE);
	if (sc->res_ioport == 0) {
		device_printf(dev, "cannot reserve I/O port range\n");
		goto error;
	}

	sc->pcf_flags = device_get_flags(dev);

	if (!(sc->pcf_flags & IIC_POLLED)) {
		sc->res_irq = bus_alloc_resource_any(dev, SYS_RES_IRQ, &sc->rid_irq,
						     RF_ACTIVE);
		if (sc->res_irq == 0) {
			device_printf(dev, "can't reserve irq, polled mode.\n");
			sc->pcf_flags |= IIC_POLLED;
		}
	}

	/* reset the chip */
	pcf_rst_card(dev, IIC_FASTEST, PCF_DEFAULT_ADDR, NULL);

	rv = bus_setup_intr(dev, sc->res_irq,
			    INTR_TYPE_NET | INTR_MPSAFE /* | INTR_ENTROPY */,
			    NULL, pcf_intr, sc, &sc->intr_cookie);
	if (rv) {
		device_printf(dev, "could not setup IRQ\n");
		goto error;
	}

	if ((sc->iicbus = device_add_child(dev, "iicbus", -1)) == NULL)
		device_printf(dev, "could not allocate iicbus instance\n");

	/* probe and attach the iicbus */
	bus_generic_attach(dev);

	return (0);

error:
	if (sc->res_irq != 0) {
		bus_release_resource(dev, SYS_RES_IRQ, sc->rid_irq,
				     sc->res_irq);
	}
	if (sc->res_ioport != 0) {
		bus_release_resource(dev, SYS_RES_MEMORY, sc->rid_ioport,
				     sc->res_ioport);
	}
	mtx_destroy(&sc->pcf_lock);
	return (rv);
}

static int
envctrl_detach(device_t dev)
{
	struct pcf_softc *sc;
	int rv;

	sc = DEVTOSOFTC(dev);

	if ((rv = bus_generic_detach(dev)) != 0)
		return (rv);

	if ((rv = device_delete_child(dev, sc->iicbus)) != 0)
		return (rv);

	if (sc->res_irq != 0) {
		bus_teardown_intr(dev, sc->res_irq, sc->intr_cookie);
		bus_release_resource(dev, SYS_RES_IRQ, sc->rid_irq, sc->res_irq);
	}

	bus_release_resource(dev, SYS_RES_MEMORY, sc->rid_ioport, sc->res_ioport);
	mtx_destroy(&sc->pcf_lock);

	return (0);
}

DRIVER_MODULE(envctrl, ebus, envctrl_driver, envctrl_devclass, 0, 0);
DRIVER_MODULE(iicbus, envctrl, iicbus_driver, iicbus_devclass, 0, 0);
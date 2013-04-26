
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

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/condvar.h>
#include <sys/conf.h>
#include <sys/bio.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/systm.h>
#include <sys/taskqueue.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <geom/geom_disk.h>

#include <dev/altera/jtag_uart/altera_jtag_uart.h>

/*
 * Nexus bus attachment for Altera JTAG UARTs.  Appropriate for most Altera
 * FPGA SoC-style configurations in which the IP core will be exposed to the
 * processor via a memory-mapped Avalon bus.
 */
static int
altera_jtag_uart_nexus_probe(device_t dev)
{

	device_set_desc(dev, "Altera JTAG UART");
	return (BUS_PROBE_DEFAULT);
}

static int
altera_jtag_uart_nexus_attach(device_t dev)
{
	struct altera_jtag_uart_softc *sc;
	int error;

	error = 0;
	sc = device_get_softc(dev);
	sc->ajus_dev = dev;
	sc->ajus_unit = device_get_unit(dev);
	sc->ajus_mem_rid = 0;
	sc->ajus_mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
	    &sc->ajus_mem_rid, RF_ACTIVE);
	if (sc->ajus_mem_res == NULL) {
		device_printf(dev, "couldn't map memory\n");
		error = ENXIO;
		goto out;
	}

	/*
	 * Interrupt support is optional -- if we can't allocate an IRQ, then
	 * we fall back on polling.
	 */
	sc->ajus_irq_rid = 0;
	sc->ajus_irq_res = bus_alloc_resource_any(dev, SYS_RES_IRQ,
	    &sc->ajus_irq_rid, RF_ACTIVE | RF_SHAREABLE);
	if (sc->ajus_irq_res == NULL)
		device_printf(dev,
		    "IRQ unavailable; selecting polled operation\n");
	error = altera_jtag_uart_attach(sc);
out:
	if (error) {
		if (sc->ajus_irq_res != NULL)
			bus_release_resource(dev, SYS_RES_IRQ,
			    sc->ajus_irq_rid, sc->ajus_irq_res);
		if (sc->ajus_mem_res != NULL)
			bus_release_resource(dev, SYS_RES_MEMORY,
			    sc->ajus_mem_rid, sc->ajus_mem_res);
	}
	return (error);
}

static int
altera_jtag_uart_nexus_detach(device_t dev)
{
	struct altera_jtag_uart_softc *sc;

	sc = device_get_softc(dev);
	KASSERT(sc->ajus_mem_res != NULL, ("%s: resources not allocated",
	    __func__));

	altera_jtag_uart_detach(sc);
	bus_release_resource(dev, SYS_RES_IRQ, sc->ajus_irq_rid,
	    sc->ajus_irq_res);
	bus_release_resource(dev, SYS_RES_MEMORY, sc->ajus_mem_rid,
	    sc->ajus_mem_res);
	return (0);
}

static device_method_t altera_jtag_uart_nexus_methods[] = {
	DEVMETHOD(device_probe,		altera_jtag_uart_nexus_probe),
	DEVMETHOD(device_attach,	altera_jtag_uart_nexus_attach),
	DEVMETHOD(device_detach,	altera_jtag_uart_nexus_detach),
	{ 0, 0 }
};

static driver_t altera_jtag_uart_nexus_driver = {
	"altera_jtag_uart",
	altera_jtag_uart_nexus_methods,
	sizeof(struct altera_jtag_uart_softc),
};

DRIVER_MODULE(altera_jtag_uart, nexus, altera_jtag_uart_nexus_driver,
    altera_jtag_uart_devclass, 0, 0);
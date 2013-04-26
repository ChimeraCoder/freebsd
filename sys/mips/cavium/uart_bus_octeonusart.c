
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

/*
 * XXXMIPS: This file is hacked from arm/... . XXXMIPS here means this file is
 * experimental and was written for MIPS32 port.
 */
#include "opt_uart.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/pci/pcivar.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

#include <mips/cavium/octeon_pcmap_regs.h>

#include <contrib/octeon-sdk/cvmx.h>

#include "uart_if.h"

extern struct uart_class uart_oct16550_class;

static int uart_octeon_probe(device_t dev);

static device_method_t uart_octeon_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, uart_octeon_probe),
	DEVMETHOD(device_attach, uart_bus_attach),
	DEVMETHOD(device_detach, uart_bus_detach),
	{0, 0}
};

static driver_t uart_octeon_driver = {
	uart_driver_name,
	uart_octeon_methods,
	sizeof(struct uart_softc),
};

extern 
SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;

static int
uart_octeon_probe(device_t dev)
{
	struct uart_softc *sc;
	int unit;

	unit = device_get_unit(dev);
	sc = device_get_softc(dev);
	sc->sc_class = &uart_oct16550_class;

	/*
	 * We inherit the settings from the systme console.  Note, the bst
	 * bad bus_space_map are bogus here, but obio doesn't yet support
	 * them, it seems.
	 */
	sc->sc_sysdev = SLIST_FIRST(&uart_sysdevs);
	bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));
	sc->sc_bas.bst = uart_bus_space_mem;
	/*
	 * XXX
	 * RBR isn't really a great base address.
	 */
	if (bus_space_map(sc->sc_bas.bst, CVMX_MIO_UARTX_RBR(0),
	    uart_getrange(sc->sc_class), 0, &sc->sc_bas.bsh) != 0)
		return (ENXIO);
	return (uart_bus_probe(dev, sc->sc_bas.regshft, 0, 0, unit));
}

DRIVER_MODULE(uart, obio, uart_octeon_driver, uart_devclass, 0, 0);
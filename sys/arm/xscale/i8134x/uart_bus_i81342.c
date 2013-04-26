
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

#include <dev/ic/ns16550.h>

#include "uart_if.h"

static int uart_i81342_probe(device_t dev);

static device_method_t uart_i81342_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_i81342_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_i81342_driver = {
	uart_driver_name,
	uart_i81342_methods,
	sizeof(struct uart_softc),
};

extern SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;
static int
uart_i81342_probe(device_t dev)
{
	struct uart_softc *sc;
	int err;

	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;
	if (device_get_unit(dev) == 0) {
		sc->sc_sysdev = SLIST_FIRST(&uart_sysdevs);
		bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));
	}
	sc->sc_rres = bus_alloc_resource(dev, SYS_RES_IOPORT, &sc->sc_rrid,
            0, ~0, uart_getrange(sc->sc_class), RF_ACTIVE);
	
	sc->sc_bas.bsh = rman_get_bushandle(sc->sc_rres);
	sc->sc_bas.bst = rman_get_bustag(sc->sc_rres);
	bus_space_write_4(sc->sc_bas.bst, sc->sc_bas.bsh, REG_IER << 2,
	    0x40 | 0x10);
        bus_release_resource(dev, sc->sc_rtype, sc->sc_rrid, sc->sc_rres);

	err = uart_bus_probe(dev, 2, 33334000, 0, device_get_unit(dev));
	sc->sc_rxfifosz = sc->sc_txfifosz = 1;
	return (err);
}


DRIVER_MODULE(uart, obio, uart_i81342_driver, uart_devclass, 0, 0);
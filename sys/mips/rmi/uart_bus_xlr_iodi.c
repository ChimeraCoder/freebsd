
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>
#include <mips/rmi/iomap.h>
#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

static int uart_iodi_probe(device_t dev);

static device_method_t uart_iodi_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, uart_iodi_probe),
	DEVMETHOD(device_attach, uart_bus_attach),
	DEVMETHOD(device_detach, uart_bus_detach),
	{0, 0}
};

static driver_t uart_iodi_driver = {
	uart_driver_name,
	uart_iodi_methods,
	sizeof(struct uart_softc),
};


extern SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;
static int
uart_iodi_probe(device_t dev)
{
	struct uart_softc *sc;
	sc = device_get_softc(dev);
	sc->sc_sysdev = SLIST_FIRST(&uart_sysdevs);
	sc->sc_class = &uart_ns8250_class;
	bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));
	sc->sc_sysdev->bas.bst = rmi_bus_space;
	sc->sc_sysdev->bas.bsh = MIPS_PHYS_TO_KSEG1(XLR_UART0ADDR);
	sc->sc_bas.bst = rmi_bus_space;
	sc->sc_bas.bsh = MIPS_PHYS_TO_KSEG1(XLR_UART0ADDR);
	/* regshft = 2, rclk = 66000000, rid = 0, chan = 0 */
	return (uart_bus_probe(dev, 2, 66000000, 0, 0));
}

DRIVER_MODULE(uart, iodi, uart_iodi_driver, uart_devclass, 0, 0);
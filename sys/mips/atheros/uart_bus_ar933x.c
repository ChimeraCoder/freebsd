
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

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>
#include <dev/uart/uart_bus.h>

#include <mips/atheros/ar71xxreg.h>
#include <mips/atheros/ar71xx_cpudef.h>

#include <mips/atheros/uart_dev_ar933x.h>

#include "uart_if.h"

static int uart_ar933x_probe(device_t dev);
extern struct uart_class uart_ar933x_uart_class;

static device_method_t uart_ar933x_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_ar933x_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_ar933x_driver = {
	uart_driver_name,
	uart_ar933x_methods,
	sizeof(struct uart_softc),
};

extern SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;

static int
uart_ar933x_probe(device_t dev)
{
	struct uart_softc *sc;
	uint64_t freq;

	freq = ar71xx_refclk();

	sc = device_get_softc(dev);
	sc->sc_sysdev = SLIST_FIRST(&uart_sysdevs);
	sc->sc_class = &uart_ar933x_class;
	bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));
	sc->sc_sysdev->bas.regshft = 0;
	sc->sc_sysdev->bas.bst = mips_bus_space_generic;
	sc->sc_sysdev->bas.bsh = MIPS_PHYS_TO_KSEG1(AR71XX_UART_ADDR);
	sc->sc_bas.regshft = 0;
	sc->sc_bas.bst = mips_bus_space_generic;
	sc->sc_bas.bsh = MIPS_PHYS_TO_KSEG1(AR71XX_UART_ADDR);

	return (uart_bus_probe(dev, 2, freq, 0, 0));
}

DRIVER_MODULE(uart, apb, uart_ar933x_driver, uart_devclass, 0, 0);
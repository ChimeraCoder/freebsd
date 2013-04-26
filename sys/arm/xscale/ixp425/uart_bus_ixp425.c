
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
#include <dev/ic/ns16550.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

#include <arm/xscale/ixp425/ixp425reg.h>
#include <arm/xscale/ixp425/ixp425var.h>

#include "uart_if.h"

static int uart_ixp425_probe(device_t dev);

static device_method_t uart_ixp425_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_ixp425_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_ixp425_driver = {
	uart_driver_name,
	uart_ixp425_methods,
	sizeof(struct uart_softc),
};
DRIVER_MODULE(uart, ixp, uart_ixp425_driver, uart_devclass, 0, 0);

static int
uart_ixp425_probe(device_t dev)
{
	struct uart_softc *sc;
	int unit = device_get_unit(dev);
	u_int rclk;

	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;
	if (resource_int_value("uart", unit, "rclk", &rclk))
		rclk = IXP425_UART_FREQ;
	if (bootverbose)
		device_printf(dev, "rclk %u\n", rclk);

	return uart_bus_probe(dev, 0, rclk, 0, 0);
}
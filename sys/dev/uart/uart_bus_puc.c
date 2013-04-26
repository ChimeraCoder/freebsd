
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

#include <dev/puc/puc_bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

static int uart_puc_probe(device_t dev);

static device_method_t uart_puc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_puc_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	/* Serdev interface */
	DEVMETHOD(serdev_ihand,		uart_bus_ihand),
	DEVMETHOD(serdev_ipend,		uart_bus_ipend),
	{ 0, 0 }
};

static driver_t uart_puc_driver = {
	uart_driver_name,
	uart_puc_methods,
	sizeof(struct uart_softc),
};

static int
uart_puc_probe(device_t dev)
{
	device_t parent;
	struct uart_softc *sc;
	uintptr_t rclk, type;

	parent = device_get_parent(dev);
	sc = device_get_softc(dev);

	if (BUS_READ_IVAR(parent, dev, PUC_IVAR_TYPE, &type))
		return (ENXIO);
	if (type != PUC_TYPE_SERIAL)
		return (ENXIO);

	sc->sc_class = &uart_ns8250_class;

	if (BUS_READ_IVAR(parent, dev, PUC_IVAR_CLOCK, &rclk))
		rclk = 0;
	return (uart_bus_probe(dev, 0, rclk, 0, 0));
}

DRIVER_MODULE(uart, puc, uart_puc_driver, uart_devclass, 0, 0);
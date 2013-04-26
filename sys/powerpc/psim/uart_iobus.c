
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
 * PSIM local bus 16550
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/tty.h>
#include <machine/bus.h>
#include <sys/timepps.h>

#include <dev/ofw/openfirm.h>
#include <powerpc/psim/iobusvar.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

static int uart_iobus_probe(device_t dev);

static device_method_t uart_iobus_methods[] = {
        /* Device interface */
	DEVMETHOD(device_probe,		uart_iobus_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),

	{ 0, 0 }
};

static driver_t uart_iobus_driver = {
	uart_driver_name,
	uart_iobus_methods,
	sizeof(struct uart_softc),
};

static int
uart_iobus_probe(device_t dev)
{
	struct uart_softc *sc;
	char *type;

	type = iobus_get_name(dev);
	if (strncmp(type, "com", 3) != 0)
		return (ENXIO);

	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;

	device_set_desc(dev, "PSIM serial port");
	return (uart_bus_probe(dev, 0, 0, 0, 0));
}

DRIVER_MODULE(uart, iobus, uart_iobus_driver, uart_devclass, 0, 0);
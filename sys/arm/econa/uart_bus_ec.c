
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

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

#include <arm/econa/econa_reg.h>

static int uart_ec_probe(device_t dev);

static device_method_t uart_ec_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,	uart_ec_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_ec_driver = {
	uart_driver_name,
	uart_ec_methods,
	sizeof(struct uart_softc),
};

static int
uart_ec_probe(device_t dev)
{
	struct	uart_softc *sc;
	int status;

	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;
	status = uart_bus_probe(dev, EC_UART_REGSHIFT, EC_UART_CLOCK, 0, 0);
	return (status);
}

DRIVER_MODULE(uart, econaarm, uart_ec_driver, uart_devclass, 0, 0);
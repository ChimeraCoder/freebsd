
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
#include <machine/resource.h>

#include <dev/pccard/pccard_cis.h>
#include <dev/pccard/pccardvar.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>

#include "pccarddevs.h"

static	int	uart_pccard_probe(device_t dev);
static	int	uart_pccard_attach(device_t dev);

static device_method_t uart_pccard_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_pccard_probe),
	DEVMETHOD(device_attach,	uart_pccard_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),

	{ 0, 0 }
};

static driver_t uart_pccard_driver = {
	uart_driver_name,
	uart_pccard_methods,
	sizeof(struct uart_softc),
};

static int
uart_pccard_probe(device_t dev)
{
	int error;
	uint32_t fcn;

	fcn = PCCARD_FUNCTION_UNSPEC;
	error = pccard_get_function(dev, &fcn);
	if (error != 0)
		return (error);
	/*
	 * If a serial card, we are likely the right driver.  However,
	 * some serial cards are better serviced by other drivers, so
	 * allow other drivers to claim it, if they want.
	 */
	if (fcn == PCCARD_FUNCTION_SERIAL)
		return (BUS_PROBE_GENERIC);

	return (ENXIO);
}

static int
uart_pccard_attach(device_t dev)
{
	struct uart_softc *sc;
	int error;

	sc = device_get_softc(dev);
	sc->sc_class = &uart_ns8250_class;

	error = uart_bus_probe(dev, 0, 0, 0, 0);
	if (error > 0)
		return (error);
	return (uart_bus_attach(dev));
}

DRIVER_MODULE(uart, pccard, uart_pccard_driver, uart_devclass, 0, 0);
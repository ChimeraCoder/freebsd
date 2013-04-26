
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
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

#include <arm/at91/at91var.h>

#include "uart_if.h"

static int usart_at91_probe(device_t dev);

extern struct uart_class at91_usart_class;

static device_method_t usart_at91_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		usart_at91_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t usart_at91_driver = {
	uart_driver_name,
	usart_at91_methods,
	sizeof(struct uart_softc),
};

extern SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;

static int
usart_at91_probe(device_t dev)
{
	struct uart_softc *sc;

	sc = device_get_softc(dev);
	switch (device_get_unit(dev))
	{
	case 0:
		device_set_desc(dev, "DBGU");
		/*
		 * Setting sc_sysdev makes this device a 'system device' and
		 * indirectly makes it the system console.
		 */
		sc->sc_sysdev = SLIST_FIRST(&uart_sysdevs);
		bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));
		break;
	case 1:
		device_set_desc(dev, "USART0");
		break;
	case 2:
		device_set_desc(dev, "USART1");
		break;
	case 3:
		device_set_desc(dev, "USART2");
		break;
	case 4:
		device_set_desc(dev, "USART3");
		break;
	case 5:
		device_set_desc(dev, "USART4");
		break;
	case 6:
		device_set_desc(dev, "USART5");
		break;
	}
	sc->sc_class = &at91_usart_class;
	if (sc->sc_class->uc_rclk == 0)
		sc->sc_class->uc_rclk = at91_master_clock;
	return (uart_bus_probe(dev, 0, 0, 0, device_get_unit(dev)));
}


DRIVER_MODULE(uart, atmelarm, usart_at91_driver, uart_devclass, 0, 0);
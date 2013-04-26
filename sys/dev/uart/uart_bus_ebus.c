
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

#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>
#include <machine/ver.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

static int uart_ebus_probe(device_t dev);

static device_method_t uart_ebus_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		uart_ebus_probe),
	DEVMETHOD(device_attach,	uart_bus_attach),
	DEVMETHOD(device_detach,	uart_bus_detach),
	{ 0, 0 }
};

static driver_t uart_ebus_driver = {
	uart_driver_name,
	uart_ebus_methods,
	sizeof(struct uart_softc),
};

static int
uart_ebus_probe(device_t dev)
{
	const char *nm, *cmpt;
	struct uart_softc *sc;
	struct uart_devinfo dummy;

	sc = device_get_softc(dev);
	sc->sc_class = NULL;

	nm = ofw_bus_get_name(dev);
	cmpt = ofw_bus_get_compat(dev);
	if (cmpt == NULL)
		cmpt = "";
	if (!strcmp(nm, "lom-console") || !strcmp(nm, "su") ||
	    !strcmp(nm, "su_pnp") || !strcmp(cmpt, "rsc-console") ||
	    !strcmp(cmpt, "rsc-control") || !strcmp(cmpt, "su") ||
	    !strcmp(cmpt, "su16550") || !strcmp(cmpt, "su16552")) {
		/*
		 * On AXi and AXmp boards the NS16550 (used to connect
		 * keyboard/mouse) share their IRQ lines with the i8042.
		 * Any IRQ activity (typically during attach) of the
		 * NS16550 used to connect the keyboard when actually the
		 * PS/2 keyboard is selected in OFW causes interaction
		 * with the OBP i8042 driver resulting in a hang and vice
		 * versa. As RS232 keyboards and mice obviously aren't
		 * meant to be used in parallel with PS/2 ones on these
		 * boards don't attach to the NS16550 in case the RS232
		 * keyboard isn't selected in order to prevent such hangs.
		 */
		if ((!strcmp(sparc64_model, "SUNW,UltraAX-MP") ||
		    !strcmp(sparc64_model, "SUNW,UltraSPARC-IIi-Engine")) &&
		    uart_cpu_getdev(UART_DEV_KEYBOARD, &dummy)) {
				device_disable(dev);
				return (ENXIO);
		}
		sc->sc_class = &uart_ns8250_class;
		return (uart_bus_probe(dev, 0, 0, 0, 0));
	}

	return (ENXIO);
}

DRIVER_MODULE(uart, ebus, uart_ebus_driver, uart_devclass, 0, 0);
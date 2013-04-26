
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

#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

bus_space_tag_t uart_bus_space_io = X86_BUS_SPACE_IO;
bus_space_tag_t uart_bus_space_mem = X86_BUS_SPACE_MEM;

int
uart_cpu_eqres(struct uart_bas *b1, struct uart_bas *b2)
{

	return ((b1->bsh == b2->bsh && b1->bst == b2->bst) ? 1 : 0);
}

int
uart_cpu_getdev(int devtype, struct uart_devinfo *di)
{
	struct uart_class *class;
	unsigned int i, ivar;

	class = &uart_ns8250_class;
	if (class == NULL)
		return (ENXIO);

	/* Check the environment. */
	if (uart_getenv(devtype, di, class) == 0)
		return (0);

	/*
	 * Scan the hints. We only try units 0 to 3 (inclusive). This
	 * covers the ISA legacy where 4 UARTs had their resources
	 * predefined.
	 */
	for (i = 0; i < 4; i++) {
		if (resource_int_value("uart", i, "flags", &ivar))
			continue;
		if (devtype == UART_DEV_CONSOLE && !UART_FLAGS_CONSOLE(ivar))
			continue;
		if (devtype == UART_DEV_DBGPORT && !UART_FLAGS_DBGPORT(ivar))
			continue;
		/*
		 * We have a possible device. Make sure it's enabled and
		 * that we have an I/O port.
		 */
		if (resource_int_value("uart", i, "disabled", &ivar) == 0 &&
		    ivar != 0)
			continue;
		if (resource_int_value("uart", i, "port", &ivar) != 0 ||
		    ivar == 0)
			continue;
		/*
		 * Got it. Fill in the instance and return it. We only have
		 * ns8250 and successors on i386.
		 */
		di->ops = uart_getops(class);
		di->bas.chan = 0;
		di->bas.bst = uart_bus_space_io;
		if (bus_space_map(di->bas.bst, ivar, uart_getrange(class), 0,
		    &di->bas.bsh) != 0)
			continue;
		di->bas.regshft = 0;
		di->bas.rclk = 0;
		if (resource_int_value("uart", i, "baud", &ivar) != 0)
			ivar = 0;
		di->baudrate = ivar;
		di->databits = 8;
		di->stopbits = 1;
		di->parity = UART_PARITY_NONE;
		return (0);
	}

	return (ENXIO);
}
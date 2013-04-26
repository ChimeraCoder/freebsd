
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
#include <sys/cons.h>
#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

#include <arm/xscale/ixp425/ixp425reg.h>
#include <arm/xscale/ixp425/ixp425var.h>

bus_space_tag_t uart_bus_space_io;
bus_space_tag_t uart_bus_space_mem;

int
uart_cpu_eqres(struct uart_bas *b1, struct uart_bas *b2)
{
	return ((b1->bsh == b2->bsh && b1->bst == b2->bst) ? 1 : 0);
}

int
uart_cpu_getdev(int devtype, struct uart_devinfo *di)
{
	uint32_t i, ivar, vaddr;

	/*
	 * Scan the hints. The IXP425 only have 2 serial ports, so only
	 * scan them.
	 */
	for (i = 0; i < 2; i++) {
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
		if (resource_int_value("uart", i, "addr", &ivar) != 0 ||
		    ivar == 0)
			continue;
		/* Got it. Fill in the instance and return it. */
		di->ops = uart_getops(&uart_ns8250_class);
		di->bas.chan = 0;
		di->bas.bst = &ixp425_a4x_bs_tag;
		di->bas.regshft = 0;
		di->bas.rclk = IXP425_UART_FREQ;
		di->baudrate = 115200;
		di->databits = 8;
		di->stopbits = 1;
		di->parity = UART_PARITY_NONE;
		uart_bus_space_io = NULL;
		uart_bus_space_mem = &ixp425_a4x_bs_tag;

		getvbase(ivar, IXP425_REG_SIZE, &vaddr);
		di->bas.bsh = vaddr;
		return (0);
	}

	return (ENXIO);
}
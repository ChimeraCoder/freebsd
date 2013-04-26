
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
 * Skeleton of this file was based on respective code for ARM
 * code written by Olivier Houchard.
 */
/*
 * XLRMIPS: This file is hacked from arm/...
 */
#include "opt_platform.h"

#ifndef	FDT			/* use FDT uart when fdt is enable */
#include "opt_uart.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/cons.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

#include <mips/nlm/hal/haldefs.h>
#include <mips/nlm/hal/iomap.h>
#include <mips/nlm/hal/mips-extns.h>
#include <mips/nlm/hal/uart.h>

#include <mips/nlm/board.h>

bus_space_tag_t uart_bus_space_io;
bus_space_tag_t uart_bus_space_mem;

int
uart_cpu_eqres(struct uart_bas *b1, struct uart_bas *b2)
{
	return (b1->bsh == b2->bsh && b1->bst == b2->bst);
}

int
uart_cpu_getdev(int devtype, struct uart_devinfo *di)
{
	di->ops = uart_getops(&uart_ns8250_class);
	di->bas.chan = 0;
	di->bas.bst = rmi_uart_bus_space;
	di->bas.bsh = nlm_get_uart_regbase(0, BOARD_CONSOLE_UART);
	
	di->bas.regshft = 2;
	/* divisor = rclk / (baudrate * 16); */
	di->bas.rclk = XLP_IO_CLK;
	di->baudrate = BOARD_CONSOLE_SPEED;
	di->databits = 8;
	di->stopbits = 1;
	di->parity = UART_PARITY_NONE;

	uart_bus_space_io = NULL;
	uart_bus_space_mem = rmi_uart_bus_space;
	return (0);
}
#endif
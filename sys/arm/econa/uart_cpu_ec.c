
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
#include <sys/cons.h>

#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

#include <sys/rman.h>

#include <arm/econa/econa_reg.h>
#include <arm/econa/econa_var.h>

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
	struct uart_class *class = &uart_ns8250_class;

	di->ops = uart_getops(class);
	di->bas.chan = 0;
	di->bas.bst = obio_tag;

	if (bus_space_map(di->bas.bst, ECONA_IO_BASE + ECONA_UART_BASE,
	    ECONA_UART_SIZE,
	    0, &di->bas.bsh) != 0) {
		return (ENXIO);
	}

	di->baudrate = 0;
	di->bas.regshft = EC_UART_REGSHIFT;
	di->bas.rclk = EC_UART_CLOCK ;
	di->databits = 8;
	di->stopbits = 1;
	di->parity = UART_PARITY_NONE;
	uart_bus_space_mem = obio_tag;
	uart_bus_space_io = NULL;

	return (0);
}
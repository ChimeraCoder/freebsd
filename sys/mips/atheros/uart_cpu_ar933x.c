
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

#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

#include <mips/atheros/ar71xxreg.h>
#include <mips/atheros/ar71xx_cpudef.h>
#include <mips/atheros/ar71xx_bus_space_reversed.h>

#include <mips/atheros/uart_dev_ar933x.h>

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
	uint64_t freq;

	freq = ar71xx_refclk();

	di->ops = uart_getops(&uart_ar933x_class);
	di->bas.chan = 0;
	di->bas.bst = ar71xx_bus_space_reversed;
	di->bas.regshft = 0;	/* We'll do "correct" dword addressing here */
	di->bas.rclk = freq;
	di->baudrate = 115200;
	di->databits = 8;
	di->stopbits = 1;

	di->parity = UART_PARITY_NONE;

	uart_bus_space_io = NULL;
	uart_bus_space_mem = ar71xx_bus_space_reversed;
	di->bas.bsh = MIPS_PHYS_TO_KSEG1(AR71XX_UART_ADDR);
	return (0);
}
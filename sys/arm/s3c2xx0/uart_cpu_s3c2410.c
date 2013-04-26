
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

#include <arm/s3c2xx0/s3c2xx0var.h>

bus_space_tag_t uart_bus_space_io;
bus_space_tag_t uart_bus_space_mem;

extern struct uart_ops uart_s3c2410_ops;

vm_offset_t s3c2410_uart_vaddr;
unsigned int s3c2410_pclk;

extern struct uart_class uart_s3c2410_class;

int
uart_cpu_eqres(struct uart_bas *b1, struct uart_bas *b2)
{
	return ((b1->bsh == b2->bsh && b1->bst == b2->bst) ? 1 : 0);
}

int
uart_cpu_getdev(int devtype, struct uart_devinfo *di)
{
	if (devtype != UART_DEV_CONSOLE)
		return (ENXIO);

	di->ops = uart_getops(&uart_s3c2410_class);
	di->bas.chan = 0;
	di->bas.bst = &s3c2xx0_bs_tag;
	di->bas.bsh = s3c2410_uart_vaddr;
	di->bas.regshft = 0;
	di->bas.rclk = s3c2410_pclk;
	di->baudrate = 115200;
	di->databits = 8;
	di->stopbits = 1;
	di->parity = UART_PARITY_NONE;
	uart_bus_space_io = &s3c2xx0_bs_tag;
	uart_bus_space_mem = NULL;
	
	return (0);
}
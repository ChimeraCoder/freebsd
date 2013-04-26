
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
#include <sys/lock.h>
#include <sys/mutex.h>
#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_bus.h>
#include <dev/uart/uart_cpu.h>

#include <arm/at91/at91var.h>

bus_space_tag_t uart_bus_space_io;
bus_space_tag_t uart_bus_space_mem;

extern struct uart_class at91_usart_class;
extern struct bus_space at91_bs_tag;

int
uart_cpu_eqres(struct uart_bas *b1, struct uart_bas *b2)
{
	return ((b1->bsh == b2->bsh && b1->bst == b2->bst) ? 1 : 0);
}

int
uart_cpu_getdev(int devtype, struct uart_devinfo *di)
{
	struct uart_class *class;

	class = &at91_usart_class;
	if (class->uc_rclk == 0 && at91_master_clock != 0)
		class->uc_rclk = at91_master_clock;
	di->ops = uart_getops(class);
	di->bas.chan = 0;
	di->bas.bst = &at91_bs_tag;
	/*
	 * XXX: Not pretty, but will work because we map the needed addresses
	 * early.  At least we probed this so that the console will work on
         * all flavors of Atmel we can detect.
	 */
	di->bas.bsh = soc_info.dbgu_base;
	di->baudrate = 115200;
	di->bas.regshft = 0;
	di->bas.rclk = 0;
	di->databits = 8;
	di->stopbits = 1;
	di->parity = UART_PARITY_NONE;
	uart_bus_space_io = &at91_bs_tag;
	uart_bus_space_mem = NULL;
	/* Check the environment for overrides */
	uart_getenv(devtype, di, class);
	return (0);
}
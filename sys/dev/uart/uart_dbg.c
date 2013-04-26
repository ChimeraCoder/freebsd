
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
#include <machine/bus.h>

#include <gdb/gdb.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>

static gdb_probe_f uart_dbg_probe;
static gdb_init_f uart_dbg_init;
static gdb_term_f uart_dbg_term;
static gdb_getc_f uart_dbg_getc;
static gdb_putc_f uart_dbg_putc;

GDB_DBGPORT(uart, uart_dbg_probe, uart_dbg_init, uart_dbg_term,
    uart_dbg_getc, uart_dbg_putc);

static struct uart_devinfo uart_dbgport;

static int
uart_dbg_probe(void)
{

	if (uart_cpu_getdev(UART_DEV_DBGPORT, &uart_dbgport))
		return (-1);

	if (uart_probe(&uart_dbgport))
		return (-1);

	return (0);
}

static void
uart_dbg_init(void)
{

	uart_dbgport.type = UART_DEV_DBGPORT;
	uart_add_sysdev(&uart_dbgport);
	uart_init(&uart_dbgport);
}

static void
uart_dbg_term(void)
{

	uart_term(&uart_dbgport);
}

static void
uart_dbg_putc(int c)
{

	uart_putc(&uart_dbgport, c);
}

static int
uart_dbg_getc(void)
{

	return (uart_poll(&uart_dbgport));
}
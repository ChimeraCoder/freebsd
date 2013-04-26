
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

/* Simple UART console driver for Allwinner A10 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cons.h>
#include <sys/consio.h>
#include <sys/kernel.h>

#ifndef	A10_UART_BASE
#define	A10_UART_BASE	0xe1c28000 	/* UART0 */
#endif

#define	REG_SHIFT	2

#define	UART_DLL	0	/* Out: Divisor Latch Low */
#define	UART_DLM	1	/* Out: Divisor Latch High */
#define	UART_FCR	2	/* Out: FIFO Control Register */
#define	UART_LCR	3	/* Out: Line Control Register */
#define	UART_MCR	4	/* Out: Modem Control Register */
#define	UART_LSR	5	/* In:  Line Status Register */
#define	UART_LSR_THRE	0x20	/* Transmit-hold-register empty */
#define	UART_LSR_DR	0x01	/* Receiver data ready */
#define	UART_MSR	6	/* In:  Modem Status Register */
#define	UART_SCR	7	/* I/O: Scratch Register */

static uint32_t
uart_getreg(uint32_t *bas)
{
	return *((volatile uint32_t *)(bas)) & 0xff;
}

static void
uart_setreg(uint32_t *bas, uint32_t val)
{
	*((volatile uint32_t *)(bas)) = val;
}

static int
ub_getc(void)
{
	while ((uart_getreg((uint32_t *)(A10_UART_BASE + 
	    (UART_LSR << REG_SHIFT))) & UART_LSR_DR) == 0);
		__asm __volatile("nop");

	return (uart_getreg((uint32_t *)A10_UART_BASE) & 0xff);
}

static void
ub_putc(unsigned char c)
{
	if (c == '\n')
		ub_putc('\r');

	while ((uart_getreg((uint32_t *)(A10_UART_BASE + 
	    (UART_LSR << REG_SHIFT))) & UART_LSR_THRE) == 0)
		__asm __volatile("nop");

	uart_setreg((uint32_t *)A10_UART_BASE, c);
}

static cn_probe_t	uart_cnprobe;
static cn_init_t	uart_cninit;
static cn_term_t	uart_cnterm;
static cn_getc_t	uart_cngetc;
static cn_putc_t	uart_cnputc;
static cn_grab_t	uart_cngrab;
static cn_ungrab_t	uart_cnungrab;

static void
uart_cngrab(struct consdev *cp)
{
}

static void
uart_cnungrab(struct consdev *cp)
{
}


static void
uart_cnprobe(struct consdev *cp)
{
	sprintf(cp->cn_name, "uart");
	cp->cn_pri = CN_NORMAL;
}

static void
uart_cninit(struct consdev *cp)
{
	uart_setreg((uint32_t *)(A10_UART_BASE + 
	    (UART_FCR << REG_SHIFT)), 0x06);
}

void
uart_cnputc(struct consdev *cp, int c)
{
	ub_putc(c);
}

int
uart_cngetc(struct consdev * cp)
{
	return ub_getc();
}

static void
uart_cnterm(struct consdev * cp)
{
}

CONSOLE_DRIVER(uart);
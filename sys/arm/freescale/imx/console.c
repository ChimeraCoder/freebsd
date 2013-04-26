
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

/* Simple UART console driver for Freescale i.MX515 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cons.h>
#include <sys/consio.h>
#include <sys/kernel.h>

/* Allow it to be predefined, to be able to use another UART for console */
#ifndef	IMX_UART_BASE
#define	IMX_UART_BASE	0xe3fbc000 /* UART1 */
#endif

#define	IMX_RXD			(u_int32_t *)(IMX_UART_BASE + 0x00)
#define	IMX_TXD			(u_int32_t *)(IMX_UART_BASE + 0x40)

#define	IMX_UFCR		(u_int32_t *)(IMX_UART_BASE + 0x90)
#define	IMX_USR1		(u_int32_t *)(IMX_UART_BASE + 0x94)
#define	IMX_USR1_TRDY		(1 << 13)

#define	IMX_USR2		(u_int32_t *)(IMX_UART_BASE + 0x98)
#define	IMX_USR2_RDR		(1 << 0)
#define	IMX_USR2_TXFE		(1 << 14)
#define	IMX_USR2_TXDC		(1 << 3)

#define	IMX_UTS			(u_int32_t *)(IMX_UART_BASE + 0xb4)
#define	IMX_UTS_TXFULL		(1 << 4)

/*
 * uart related funcs
 */
static u_int32_t
uart_getreg(u_int32_t *bas)
{

	return *((volatile u_int32_t *)(bas)) & 0xff;
}

static void
uart_setreg(u_int32_t *bas, u_int32_t val)
{

	*((volatile u_int32_t *)(bas)) = (u_int32_t)val;
}

static int
ub_tstc(void)
{

	return ((uart_getreg(IMX_USR2) & IMX_USR2_RDR) ? 1 : 0);
}

static int
ub_getc(void)
{

	while (!ub_tstc());
		__asm __volatile("nop");

	return (uart_getreg(IMX_RXD) & 0xff);
}

static void
ub_putc(unsigned char c)
{

	if (c == '\n')
		ub_putc('\r');

	while (uart_getreg(IMX_UTS) & IMX_UTS_TXFULL)
		__asm __volatile("nop");

	uart_setreg(IMX_TXD, c);
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
	uart_setreg(IMX_UFCR, 0x00004210);
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
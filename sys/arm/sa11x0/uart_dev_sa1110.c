
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
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/tty.h>
#include <machine/bus.h>

#include <dev/uart/uart.h>
#include <dev/uart/uart_cpu.h>
#include <dev/uart/uart_bus.h>
#include <arm/sa11x0/sa11x0_reg.h>
#include <arm/sa11x0/uart_dev_sa1110.h>

#include "uart_if.h"

#define      DEFAULT_RCLK    3686400

/*
 * Low-level UART interface.
 */
static int sa1110_probe(struct uart_bas *bas);
static void sa1110_init(struct uart_bas *bas, int, int, int, int);
static void sa1110_term(struct uart_bas *bas);
static void sa1110_putc(struct uart_bas *bas, int);
static int sa1110_rxready(struct uart_bas *bas);
static int sa1110_getc(struct uart_bas *bas, struct mtx *mtx);

extern SLIST_HEAD(uart_devinfo_list, uart_devinfo) uart_sysdevs;

static struct uart_ops uart_sa1110_ops = {
	.probe = sa1110_probe,
	.init = sa1110_init,
	.term = sa1110_term,
	.putc = sa1110_putc,
	.rxready = sa1110_rxready,
	.getc = sa1110_getc,
};

static int
sa1110_probe(struct uart_bas *bas)
{
	return (0);
}

static void
sa1110_init(struct uart_bas *bas, int baudrate, int databits, int stopbits,
    int parity)
{
	int brd;
	
	if (bas->rclk == 0)
		bas->rclk = DEFAULT_RCLK;
	while (uart_getreg(bas, SACOM_SR1) & SR1_TBY);
	uart_setreg(bas, SACOM_CR3, 0);
	brd = SACOMSPEED(baudrate);
	uart_setreg(bas, SACOM_CR1, brd >> 8);
	uart_setreg(bas, SACOM_CR2, brd & 0xff);
	uart_setreg(bas, SACOM_CR3, CR3_RXE | CR3_TXE);
}

static void
sa1110_term(struct uart_bas *bas)
{
	/* XXX */
}

static void
sa1110_putc(struct uart_bas *bas, int c)
{
	while (!(uart_getreg(bas, SACOM_SR1) & SR1_TNF));
	uart_setreg(bas, SACOM_DR, c);
}

static int
sa1110_rxready(struct uart_bas *bas)
{

	return ((uart_getreg(bas, SACOM_SR1) & SR1_RNE) != 0 ? 1 : 0);
}

static int
sa1110_getc(struct uart_bas *bas, struct mtx *mtx)
{
	int c;

	while (!(uart_getreg(bas, SACOM_SR1) & SR1_RNE)) {
		u_int32_t sr0;

		sr0 = uart_getreg(bas, SACOM_SR0);
		if (ISSET(sr0, SR0_RBB))
			uart_setreg(bas, SACOM_SR0, SR0_RBB);
		if (ISSET(sr0, SR0_REB))
			uart_setreg(bas, SACOM_SR0, SR0_REB);
	}
	c = uart_getreg(bas, SACOM_DR);
	c &= 0xff;
	return (c);
}

static int sa1110_bus_probe(struct uart_softc *sc);
static int sa1110_bus_attach(struct uart_softc *sc);
static int sa1110_bus_flush(struct uart_softc *, int);
static int sa1110_bus_getsig(struct uart_softc *);
static int sa1110_bus_ioctl(struct uart_softc *, int, intptr_t);
static int sa1110_bus_ipend(struct uart_softc *);
static int sa1110_bus_param(struct uart_softc *, int, int, int, int);
static int sa1110_bus_receive(struct uart_softc *);
static int sa1110_bus_setsig(struct uart_softc *, int);
static int sa1110_bus_transmit(struct uart_softc *);

static kobj_method_t sa1110_methods[] = {
	KOBJMETHOD(uart_probe,		sa1110_bus_probe),
	KOBJMETHOD(uart_attach, 	sa1110_bus_attach),
	KOBJMETHOD(uart_flush,		sa1110_bus_flush),
	KOBJMETHOD(uart_getsig,		sa1110_bus_getsig),
	KOBJMETHOD(uart_ioctl,		sa1110_bus_ioctl),
	KOBJMETHOD(uart_ipend,		sa1110_bus_ipend),
	KOBJMETHOD(uart_param,		sa1110_bus_param),
	KOBJMETHOD(uart_receive,	sa1110_bus_receive),
	KOBJMETHOD(uart_setsig,		sa1110_bus_setsig),
	KOBJMETHOD(uart_transmit,	sa1110_bus_transmit),
	
	{0, 0 }
};

int
sa1110_bus_probe(struct uart_softc *sc)
{
	sc->sc_txfifosz = 3;
	sc->sc_rxfifosz = 1;
	return (0);
}

static int
sa1110_bus_attach(struct uart_softc *sc)
{
	 bcopy(&sc->sc_sysdev->bas, &sc->sc_bas, sizeof(sc->sc_bas));

	 sc->sc_hwiflow = 0;
	 uart_setreg(&sc->sc_bas, SACOM_CR3, CR3_RXE | CR3_TXE | CR3_RIE | CR3_TIE);
	return (0);
}
static int
sa1110_bus_transmit(struct uart_softc *sc)
{
	int i;
#if 0
	int sr = uart_getreg(&sc->sc_bas, SACOM_SR0);

	while (!(uart_getreg(&sc->sc_bas, SACOM_CR3) & CR3_TIE))
		uart_setreg(&sc->sc_bas, SACOM_CR3,
		    uart_getreg(&sc->sc_bas, SACOM_CR3) | CR3_TIE);
#endif

	sc->sc_txbusy = 1;
	uart_setreg(&sc->sc_bas, SACOM_CR3, uart_getreg(&sc->sc_bas, SACOM_CR3)
	    | CR3_TIE);
	for (i = 0; i < sc->sc_txdatasz; i++) {
		while (!(uart_getreg(&sc->sc_bas, SACOM_SR1) & SR1_TNF));

		uart_setreg(&sc->sc_bas, SACOM_DR, sc->sc_txbuf[i]);
		uart_barrier(&sc->sc_bas);
	}
#if 0
	sr = uart_getreg(&sc->sc_bas, SACOM_SR0);
#endif

	return (0);
}
static int
sa1110_bus_setsig(struct uart_softc *sc, int sig)
{
	return (0);
}
static int
sa1110_bus_receive(struct uart_softc *sc)
{
	
#if 0
	while (!(uart_getreg(&sc->sc_bas, SACOM_SR1) & SR1_RNE)) {
		u_int32_t sr0;

		sr0 = uart_getreg(&sc->sc_bas, SACOM_SR0);
		if (ISSET(sr0, SR0_RBB))
			uart_setreg(&sc->sc_bas, SACOM_SR0, SR0_RBB);
		if (ISSET(sr0, SR0_REB))
			uart_setreg(&sc->sc_bas, SACOM_SR0, SR0_REB);
	}
#endif
	
	uart_setreg(&sc->sc_bas, SACOM_CR3, uart_getreg(&sc->sc_bas, SACOM_CR3)
	    | CR3_RIE);
	uart_rx_put(sc, uart_getreg(&sc->sc_bas, SACOM_DR));
	return (0);
}
static int
sa1110_bus_param(struct uart_softc *sc, int baudrate, int databits,
    int stopbits, int parity)
{
	int brd;
	
	if (baudrate > 0) {
		brd = SACOMSPEED(baudrate);
		uart_setreg(&sc->sc_bas, SACOM_CR1, brd >> 8);
		uart_setreg(&sc->sc_bas, SACOM_CR2, brd & 0xff);
	}
	return (0);
}
static int
sa1110_bus_ipend(struct uart_softc *sc)
{
	int sr = uart_getreg(&sc->sc_bas, SACOM_SR0);
	int ipend = 0;
	int mask = CR3_RIE | CR3_TIE;
	if (sr & 1) {
		if (uart_getreg(&sc->sc_bas, SACOM_CR3) & CR3_TIE)
			ipend |= SER_INT_TXIDLE;
		mask &= ~CR3_TIE;
	}
	if (sr & 4) {
		if (uart_getreg(&sc->sc_bas, SACOM_CR3) & CR3_RIE)
			ipend |= SER_INT_RXREADY;
		mask &= ~CR3_RIE;
	}
	uart_setreg(&sc->sc_bas, SACOM_CR3, CR3_RXE | mask);
	return (ipend);
}
static int
sa1110_bus_flush(struct uart_softc *sc, int what)
{
	return (0);
}

static int
sa1110_bus_getsig(struct uart_softc *sc)
{
	return (0);
}

static int
sa1110_bus_ioctl(struct uart_softc *sc, int request, intptr_t data)
{
	return (EINVAL);
}

struct uart_class uart_sa1110_class = {
	"sa1110",
	sa1110_methods,
	1,
	.uc_ops = &uart_sa1110_ops,
	.uc_range = 8,
	.uc_rclk = 3686400
};

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
 * Ethernut 5 board support
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <machine/board.h>
#include <arm/at91/at91_pioreg.h>
#include <arm/at91/at91_piovar.h>
#include <arm/at91/at91board.h>
#include <arm/at91/at91sam9260reg.h>

BOARD_INIT long
board_init(void)
{

	/*
	 * DBGU
	 */
	/* DRXD */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB14, 0);
	/* DTXD */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB15, 1);

	/*
	 * EMAC
	 */
	/* ETX0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA12, 0);
	/* ETX1 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA13, 0);
	/* ERX0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA14, 0);
	/* ERX1 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA15, 0);
	/* ETXEN */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA16, 0);
	/* ERXDV */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA17, 0);
	/* ERXER */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA18, 0);
	/* ETXCK */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA19, 0);
	/* EMDC */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA20, 0);
	/* EMDIO */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA21, 0);

	/*
	 * MMC
	 */
	/* MCDA0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA6, 1);
	/* MCCDA */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA7, 1);
	/* MCCK */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA8, 1);
	/* MCDA1 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA9, 1);
	/* MCDA2 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA10, 1);
	/* MCDA3 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA11, 1);

	/*
	 * SPI0
	 */
	/* MISO */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA0, 0);
	/* MOSI */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA1, 0);
	/* SPCK */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA2, 0);
	/* NPCS0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA3, 0);

	/*
	 * TWI
	 */
	/* TWD */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA23, 1);
	/* TWCK */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA24, 1);

	/*
	 * USART0
	 */
	/* TXD0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB4, 1);
	/* RXD0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB5, 0);
	/* DSR0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB22, 0);
	/* DCD0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB23, 0);
	/* DTR0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB24, 1);
	/* RI0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB25, 0);
	/* RTS0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB26, 1);
	/* CTS0 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB27, 0);

	/*
	 * USART2
	 */
	/* RTS2 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA4, 1);
	/* CTS2 */
	at91_pio_use_periph_a(AT91SAM9260_PIOA_BASE, AT91C_PIO_PA5, 0);
	/* TXD2 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB8, 1);
	/* RXD2 */
	at91_pio_use_periph_a(AT91SAM9260_PIOB_BASE, AT91C_PIO_PB9, 0);

	return (at91_ramsize());
}

ARM_BOARD(ETHERNUT5, "Ethernut 5")
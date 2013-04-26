
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
 * This board file can be used for both:
 * SAM9X26EK board
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");
#include <sys/param.h>
#include <sys/systm.h>

#include <machine/board.h>
#include <arm/at91/at91board.h>
#include <arm/at91/at91reg.h>
#include <arm/at91/at91var.h>
#include <arm/at91/at91sam9g20reg.h>
#include <arm/at91/at91_piovar.h>
#include <arm/at91/at91_pio_sam9g20.h>
//#include <arm/at91/at91_led.h>

BOARD_INIT long
board_init(void)
{
#if 0
	/* PIOB's A periph: Turn USART 0's TX/RX pins */
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB14_DRXD, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB15_DTXD, 1);

	/* PIOB's A periph: Turn USART 0's TX/RX pins */
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB4_TXD0, 1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB5_RXD0, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB22_DSR0, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB23_DCD0, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB24_DTR0, 1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB25_RI0, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB26_RTS0, 1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB27_CTS0, 0);

	/* PIOB's A periph: Turn USART 1's TX/RX pins */
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB6_TXD1, 1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB7_RXD1, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB28_RTS1, 1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOB_BASE, AT91C_PB29_CTS1, 0);

	/*  TWI Two-wire Serial Data */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA23_TWD,  1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA24_TWCK, 1);

#if 1
	/* 
	 * Turn off Clock to DataFlash, conflicts with MCI clock.
	 */
	at91_pio_use_gpio(AT91SAM9G20_PIOA_BASE,AT91C_PIO_PA2);
	at91_pio_gpio_input(AT91SAM9G20_PIOA_BASE,AT91C_PIO_PA2);

	/* Turn off chip select to DataFlash */
	at91_pio_gpio_output(AT91SAM9G20_PIOC_BASE,AT91C_PIO_PC11, 0);
	at91_pio_gpio_set(AT91SAM9G20_PIOC_BASE,AT91C_PIO_PC11);
	at91_pio_use_gpio(AT91SAM9G20_PIOC_BASE,AT91C_PIO_PC11);

	/*  Multimedia Card  */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA0_MCDB0, 1);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA1_MCCDB, 1);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA3_MCDB3, 1);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA4_MCDB2, 1);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA5_MCDB1, 1);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA8_MCCK,  1);
	at91_pio_use_gpio(AT91SAM9G20_PIOC_BASE, AT91C_PIO_PC9);
#else
	/* SPI0 to DataFlash */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA0, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA1, 0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA2, 0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOC_BASE, AT91C_PIO_PC11,0);

	at91_pio_gpio_input(AT91SAM9G20_PIOA_BASE,AT91C_PIO_PA8);
	at91_pio_use_gpio(AT91SAM9G20_PIOA_BASE,AT91C_PIO_PA8);
#endif

	/* EMAC */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA12_ETX0 ,  0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA13_ETX1,   0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA14_ERX0,   0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA15_ERX1,   0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA16_ETXEN,  0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA17_ERXDV,  0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA18_ERXER,  0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA19_ETXCK,  0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA20_EMDC,   0);
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE,AT91C_PA21_EMDIO,  0);

	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA10_ETX2_0, 0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA11_ETX3_0, 0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA22_ETXER,  0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA25_ERX2,   0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA26_ERX3,   0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA27_ERXCK,  0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA28_ECRS,   0);
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE,AT91C_PA29_ECOL,   0);
#endif
	return (at91_ramsize());
}

ARM_BOARD(AT91SAM9X5EK, "Atmel AT91SAM9x-EK Evaluation Board");
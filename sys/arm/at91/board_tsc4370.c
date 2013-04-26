
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

#include <machine/board.h>
#include <arm/at91/at91board.h>
#include <arm/at91/at91var.h>
#include <arm/at91/at91rm92reg.h>
#include <arm/at91/at91rm9200var.h>
#include <arm/at91/at91_piovar.h>
#include <arm/at91/at91_pioreg.h>

BOARD_INIT long
board_init(void)
{

	at91rm9200_set_subtype(AT91_ST_RM9200_PQFP);

	at91rm9200_config_uart(AT91_ID_DBGU, 0, 0);   /* DBGU just Tx and Rx */
	at91rm9200_config_uart(AT91RM9200_ID_USART0, 1, 0);   /* Tx and Rx */
	at91rm9200_config_uart(AT91RM9200_ID_USART1, 2, 0);   /* Tx and Rx */
	at91rm9200_config_uart(AT91RM9200_ID_USART2, 3, 0);   /* Tx and Rx */
	at91rm9200_config_uart(AT91RM9200_ID_USART3, 4, 0);   /* Tx and Rx */

	at91rm9200_config_mci(0);			/* tsc4370 board has only 1 wire */
							/* Newer boards may have 4 wires */

	/* Configure TWI */
	/* Configure SPI + dataflash */
	/* Configure SSC */
	/* Configure USB Host */
	/* Configure FPGA attached to chip selects */

	/* Pin assignment */
	/* Assert PA24 low -- talk to rubidium */
	at91_pio_use_gpio(AT91RM92_PIOA_BASE, AT91C_PIO_PA24);
	at91_pio_gpio_output(AT91RM92_PIOA_BASE, AT91C_PIO_PA24, 0);
	at91_pio_gpio_clear(AT91RM92_PIOA_BASE, AT91C_PIO_PA24);
	at91_pio_use_gpio(AT91RM92_PIOB_BASE,
	    AT91C_PIO_PB16 | AT91C_PIO_PB17 | AT91C_PIO_PB18 | AT91C_PIO_PB19);

	return (at91_ramsize());
}

ARM_BOARD(NONE, "TSC4370 Controller Board");
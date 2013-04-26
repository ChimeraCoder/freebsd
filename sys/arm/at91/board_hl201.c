
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
#include <arm/at91/at91sam9g20reg.h>
#include <arm/at91/at91_piovar.h>
#include <arm/at91/at91_pio_sam9g20.h>

BOARD_INIT long
board_init(void)
{
	/* Setup Ethernet Pins */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, 1<<7, 0);

	at91_pio_gpio_input(AT91SAM9G20_PIOA_BASE, 1<<7);
	at91_pio_gpio_set_deglitch(AT91SAM9G20_PIOA_BASE, 1<<7, 1);

	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA19, 0);	/* ETXCK_EREFCK */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA17, 0);	/* ERXDV */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA14, 0);	/* ERX0 */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA15, 0);	/* ERX1 */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA18, 0);	/* ERXER */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA16, 0);	/* ETXEN */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA12, 0);	/* ETX0 */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA13, 0);	/* ETX1 */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA21, 0);	/* EMDIO */
	at91_pio_use_periph_a(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA20, 0);	/* EMDC */

	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA28, 0);	/* ECRS */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA29, 0);	/* ECOL */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA25, 0);	/* ERX2 */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA26, 0);	/* ERX3 */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA27, 0);	/* ERXCK */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA23, 0);	/* ETX2 */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA24, 0);	/* ETX3 */
	at91_pio_use_periph_b(AT91SAM9G20_PIOA_BASE, AT91C_PIO_PA22, 0);	/* ETXER */

	return (at91_ramsize());
}

ARM_BOARD(NONE, "HOTe 201");
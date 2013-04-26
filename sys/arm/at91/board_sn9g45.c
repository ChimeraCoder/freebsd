
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
 * DesignA Electronics Snapper9g45
 * http://www.designa-electronics.com/
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");
#include <sys/param.h>
#include <sys/systm.h>

#include <machine/board.h>
#include <arm/at91/at91board.h>
#include <arm/at91/at91reg.h>
#include <arm/at91/at91var.h>
#include <arm/at91/at91sam9g45reg.h>
#include <arm/at91/at91_piovar.h>
#include <arm/at91/at91_pio_sam9g45.h>

BOARD_INIT long
board_init(void)
{

	/* PIOB's A periph: Turn the debug USART's TX/RX pins */
	at91_pio_use_periph_a(AT91SAM9G45_PIOB_BASE, AT91C_PB12_DRXD, 0);
	at91_pio_use_periph_a(AT91SAM9G45_PIOB_BASE, AT91C_PB13_DTXD, 1);

	return (at91_ramsize());
}

ARM_BOARD(SNAPPER9G45, "DesignA Electronics Snapper9G45");
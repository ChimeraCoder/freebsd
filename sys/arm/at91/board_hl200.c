
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

BOARD_INIT long
board_init(void)
{

	at91rm9200_set_subtype(AT91_ST_RM9200_BGA);

	/*
	 * Unsure what all is in the HOTe HL200, but I do know there's
	 * one serial port that isn't DBGU.  There's many other peripherals
	 * that need to be configured here.
	 */
	at91rm9200_config_uart(AT91_ID_DBGU, 0, 0);   /* DBGU just Tx and Rx */
	at91rm9200_config_uart(AT91RM9200_ID_USART0, 1, 0);   /* Tx and Rx */

	at91rm9200_config_mci(0);			/* HOTe HL200 unknown 1 vs 4 wire */

	/* Enable CF card slot */
	/* Enable sound thing */
	/* Enable VGA chip */
	/* Enable ethernet */
	/* Enable TWI + RTC */
	/* Enable USB Host */
	/* Enable USB Device (gadget) */

	return (at91_ramsize());
}

ARM_BOARD(NONE, "HOTe 200");
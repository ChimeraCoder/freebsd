
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
#include <arm/at91/at91rm9200var.h>

BOARD_INIT long
board_init(void)
{

	at91rm9200_set_subtype(AT91_ST_RM9200_BGA);

	/*
	 * I don't know anything at all about this board.
	 */
	at91rm9200_config_uart(AT91_ID_DBGU, 0, 0);   /* DBGU just Tx and Rx */

	at91rm9200_config_mci(0);
	/* Configure ethernet */

	return (at91_ramsize());
}

ARM_BOARD(NONE, "BWCT special");
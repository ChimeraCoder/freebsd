
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

#include "at91rm9200.h"
#include "at91rm9200_lowlevel.h"
#include "lib.h"

/*
 * int getc(int seconds)
 * 
 * Reads a character from the DBGU port, if one is available within about
 * seconds seconds.  It assumes that DBGU has already been initialized.
 */
int
getc(int seconds)
{
	AT91PS_USART pUSART = (AT91PS_USART)AT91C_BASE_DBGU;
	unsigned	thisSecond;

	// Clamp to 20s
	if (seconds > 20)
	    seconds = 20;
	thisSecond = GetSeconds();
	seconds = thisSecond + seconds;
	do {
		if ((pUSART->US_CSR & AT91C_US_RXRDY))
			return (pUSART->US_RHR & 0xFF);
		thisSecond = GetSeconds();
	} while (thisSecond != seconds);
	return (-1);
}
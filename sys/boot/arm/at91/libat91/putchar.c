
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
 * void putchar(int ch)
 * Writes a character to the DBGU port.  It assumes that DBGU has
 * already been initialized.
 */
void
putchar(int ch)
{
	AT91PS_USART pUSART = (AT91PS_USART)AT91C_BASE_DBGU;

	while (!(pUSART->US_CSR & AT91C_US_TXRDY))
		continue;
	pUSART->US_THR = (ch & 0xFF);
}

void
xputchar(int ch)
{
    if (ch == '\n')
	putchar('\r');
    putchar(ch);
}

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
#include "spi_flash.h"
#include "lib.h"

void
Delay(int us)
{
	unsigned later, now;

	now = AT91C_BASE_ST->ST_CRTR;
	later = (now + us / 25 + 1) & AT91C_ST_CRTV;
	while (later != AT91C_BASE_ST->ST_CRTR)
		continue;
}
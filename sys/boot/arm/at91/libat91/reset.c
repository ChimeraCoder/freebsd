
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
#include "lib.h"

/*
 * void reset()
 * 
 * Forces a reset of the system.  Uses watchdog timer of '1', which
 * corresponds to 128 / SLCK seconds (SLCK is 32,768 Hz, so 128/32768 is
 * 1 / 256 ~= 5.4ms
 */
void
reset(void)
{
	// The following should effect a reset.
	AT91C_BASE_ST->ST_WDMR = 1 | AT91C_ST_RSTEN;
	AT91C_BASE_ST->ST_CR = AT91C_ST_WDRST;
}

/*
 * void start_wdog()
 *
 * Starts a watchdog timer.  We force the boot process to get to the point
 * it can kick the watch dog part of the ST part for the OS's driver.
 */
void
start_wdog(int n)
{
	// The following should effect a reset after N seconds.
	AT91C_BASE_ST->ST_WDMR = (n * (32768 / 128)) | AT91C_ST_RSTEN;
	AT91C_BASE_ST->ST_CR = AT91C_ST_WDRST;
}
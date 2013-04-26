
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
#include "at91rm9200_lowlevel.h"

typedef void fn_t(void);

int
main(void)
{
	char *addr = (char *)SDRAM_BASE + (1 << 20); /* Load to base + 1MB */
	fn_t *fn = (fn_t *)(SDRAM_BASE + (1 << 20)); /* Load to base + 1MB */

	while (xmodem_rx(addr) == -1)
		continue;
	fn();
	return (0);
}
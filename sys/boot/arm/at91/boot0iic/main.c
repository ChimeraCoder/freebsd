
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

int
main(void)
{
	char *addr = (char *)SDRAM_BASE + (1 << 20); /* Load to base + 1MB */
	int len, sec;

	printf("\nSend data to be written into EEPROM\n");
	while ((len = xmodem_rx(addr)) == -1)
		continue;
	sec = GetSeconds() + 1;
	while (sec >= GetSeconds())
		continue;
	printf("\nWriting EEPROM from 0x%x to addr 0, 0x%x bytes\n", addr,
	    len);
	InitEEPROM();
	printf("init done\n");
	WriteEEPROM(0, addr, len);
	printf("\nWrote %d bytes.  Press reset\n", len);
	return (1);
}
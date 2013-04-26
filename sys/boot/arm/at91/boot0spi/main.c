
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
#include "spi_flash.h"

#define OFFSET 0

void
main(void)
{
	int len, i, j, off, sec;
	char *addr = (char *)SDRAM_BASE + (1 << 20); /* download at + 1MB */
	char *addr2 = (char *)SDRAM_BASE + (2 << 20); /* readback to + 2MB */

	SPI_InitFlash();
	printf("Waiting for data\n");
	while ((len = xmodem_rx(addr)) == -1)
		continue;
	printf("Writing %u bytes at %u\n", len, OFFSET);
	for (i = 0; i < len; i+= FLASH_PAGE_SIZE) {
		off = i + OFFSET;
		for (j = 0; j < 10; j++) {
			SPI_WriteFlash(off, addr + i, FLASH_PAGE_SIZE);
			SPI_ReadFlash(off, addr2 + i, FLASH_PAGE_SIZE);
			if (p_memcmp(addr + i, addr2 + i, FLASH_PAGE_SIZE) == 0)
				break;
		}
		if (j >= 10)
			printf("Bad Readback at %u\n", i);
	}
	sec = GetSeconds() + 2;
	while (sec <= GetSeconds())
	    continue;
	printf("Done\n");
	reset();
}
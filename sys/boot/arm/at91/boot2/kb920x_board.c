
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

#include "emac.h"
#include "lib.h"
#include "board.h"
#include "sd-card.h"

unsigned char mac[6] = { 0x42, 0x53, 0x44, 0, 0, 1 };

static void
MacFromEE()
{	
	uint32_t sig;
	sig = 0;
	ReadEEPROM(12 * 1024, (uint8_t *)&sig, sizeof(sig));
	if (sig != 0x92021054)
		return;
	ReadEEPROM(12 * 1024 + 4, mac, 6);
	printf("MAC %x:%x:%x:%x:%x:%x\n", mac[0],
	  mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void
board_init(void)
{
    InitEEPROM();
    MacFromEE();
    EMAC_Init();
    EMAC_SetMACAddress(mac);
    while (sdcard_init() == 0)
	printf("Looking for SD card\n");
}

int
drvread(void *buf, unsigned lba, unsigned nblk)
{
    return (MCI_read((char *)buf, lba << 9, nblk << 9));
}
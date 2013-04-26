
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

#include "krb5_locl.h"

static u_long table[256];

#define CRC_GEN 0xEDB88320L

void
_krb5_crc_init_table(void)
{
    static int flag = 0;
    unsigned long crc, poly;
    unsigned int i, j;

    if(flag) return;
    poly = CRC_GEN;
    for (i = 0; i < 256; i++) {
	crc = i;
	for (j = 8; j > 0; j--) {
	    if (crc & 1) {
		crc = (crc >> 1) ^ poly;
	    } else {
		crc >>= 1;
	    }
	}
	table[i] = crc;
    }
    flag = 1;
}

uint32_t
_krb5_crc_update (const char *p, size_t len, uint32_t res)
{
    while (len--)
	res = table[(res ^ *p++) & 0xFF] ^ (res >> 8);
    return res & 0xFFFFFFFF;
}
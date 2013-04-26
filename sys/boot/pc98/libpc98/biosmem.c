
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

/*
 * Obtain memory configuration information from the BIOS
 */
#include <stand.h>
#include "libi386.h"
#include "btxv86.h"

vm_offset_t	memtop, memtop_copyin, high_heap_base;
uint32_t	bios_basemem, bios_extmem, high_heap_size;

/*
 * The minimum amount of memory to reserve in bios_extmem for the heap.
 */
#define	HEAP_MIN	(3 * 1024 * 1024)

void
bios_getmem(void)
{

    bios_basemem = ((*(u_char *)PTOV(0xA1501) & 0x07) + 1) * 128 * 1024;
    bios_extmem = *(u_char *)PTOV(0xA1401) * 128 * 1024 +
	*(u_int16_t *)PTOV(0xA1594) * 1024 * 1024;

    /* Set memtop to actual top of memory */
    memtop = memtop_copyin = 0x100000 + bios_extmem;

    /*
     * If we have extended memory, use the last 3MB of 'extended' memory
     * as a high heap candidate.
     */
    if (bios_extmem >= HEAP_MIN) {
	high_heap_size = HEAP_MIN;
	high_heap_base = memtop - HEAP_MIN;
    }
}    
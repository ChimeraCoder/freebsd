
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

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/







/**
 * @file
 *
 * Interface to the EBT3000 specific devices
 *
 * <hr>$Revision: 70030 $<hr>
 *
 */

#if !defined(__FreeBSD__) || !defined(_KERNEL)
#include "cvmx-config.h"
#endif
#include "cvmx.h"
#include "cvmx-ebt3000.h"
#include "cvmx-sysinfo.h"


void ebt3000_char_write(int char_position, char val)
{
    /* Note: phys_to_ptr won't work here, as we are most likely going to access the boot bus. */
    char *led_base = CASTPTR(char , CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, cvmx_sysinfo_get()->led_display_base_addr));
    if (!led_base)
        return;
    if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_EBT3000 && cvmx_sysinfo_get()->board_rev_major == 1)
    {
        /* Rev 1 board */
        char *ptr = (char *)(led_base + 4);
        char_position &= 0x3;  /* only 4 chars */
        ptr[3 - char_position] = val;
    }
    else
    {
        /* rev 2 or later board */
        char *ptr = (char *)(led_base);
        char_position &= 0x7;  /* only 8 chars */
        ptr[char_position] = val;
    }
}

void ebt3000_str_write(const char *str)
{
    /* Note: phys_to_ptr won't work here, as we are most likely going to access the boot bus. */
    char *led_base;
    if (!cvmx_sysinfo_get()->led_display_base_addr)
        return;
    led_base = CASTPTR(char, CVMX_ADD_SEG32(CVMX_MIPS32_SPACE_KSEG0, cvmx_sysinfo_get()->led_display_base_addr));
    if (cvmx_sysinfo_get()->board_type == CVMX_BOARD_TYPE_EBT3000 && cvmx_sysinfo_get()->board_rev_major == 1)
    {
        char *ptr = (char *)(led_base + 4);
        int i;
        for (i=0; i<4; i++)
        {
            if (*str)
                ptr[3 - i] = *str++;
            else
                ptr[3 - i] = ' ';
        }
    }
    else
    {
        /* rev 2 board */
        char *ptr = (char *)(led_base);
        int i;
        for (i=0; i<8; i++)
        {
            if (*str)
                ptr[i] = *str++;
            else
                ptr[i] = ' ';
        }
    }
}
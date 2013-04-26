
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

#include "int_lib.h"

/* Returns: count of 1 bits */

COMPILER_RT_ABI si_int
__popcountsi2(si_int a)
{
    su_int x = (su_int)a;
    x = x - ((x >> 1) & 0x55555555);
    /* Every 2 bits holds the sum of every pair of bits */
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    /* Every 4 bits holds the sum of every 4-set of bits (3 significant bits) */
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    /* Every 8 bits holds the sum of every 8-set of bits (4 significant bits) */
    x = (x + (x >> 16));
    /* The lower 16 bits hold two 8 bit sums (5 significant bits).*/
    /*    Upper 16 bits are garbage */
    return (x + (x >> 8)) & 0x0000003F;  /* (6 significant bits) */
}
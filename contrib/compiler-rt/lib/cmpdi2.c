
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

/* Returns: if (a <  b) returns 0
*           if (a == b) returns 1
*           if (a >  b) returns 2
*/

COMPILER_RT_ABI si_int
__cmpdi2(di_int a, di_int b)
{
    dwords x;
    x.all = a;
    dwords y;
    y.all = b;
    if (x.s.high < y.s.high)
        return 0;
    if (x.s.high > y.s.high)
        return 2;
    if (x.s.low < y.s.low)
        return 0;
    if (x.s.low > y.s.low)
        return 2;
    return 1;
}

#ifdef __ARM_EABI__
/* Returns: if (a <  b) returns -1
*           if (a == b) returns  0
*           if (a >  b) returns  1
*/
COMPILER_RT_ABI si_int
__aeabi_lcmp(di_int a, di_int b)
{
	return __cmpdi2(a, b) - 1;
}
#endif
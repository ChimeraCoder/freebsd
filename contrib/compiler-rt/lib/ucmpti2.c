
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

#if __x86_64

/* Returns:  if (a <  b) returns 0
 *           if (a == b) returns 1
 *           if (a >  b) returns 2
 */

si_int
__ucmpti2(tu_int a, tu_int b)
{
    utwords x;
    x.all = a;
    utwords y;
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

#endif
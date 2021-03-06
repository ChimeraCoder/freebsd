
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

/* Returns: 1 if number of bits is odd else returns 0 */

si_int COMPILER_RT_ABI __paritysi2(si_int a);

COMPILER_RT_ABI si_int
__paritydi2(di_int a)
{
    dwords x;
    x.all = a;
    return __paritysi2(x.s.high ^ x.s.low);
}
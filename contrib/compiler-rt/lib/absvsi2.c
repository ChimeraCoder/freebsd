
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

/* Returns: absolute value */

/* Effects: aborts if abs(x) < 0 */

COMPILER_RT_ABI si_int
__absvsi2(si_int a)
{
    const int N = (int)(sizeof(si_int) * CHAR_BIT);
    if (a == (1 << (N-1)))
        compilerrt_abort();
    const si_int t = a >> (N - 1);
    return (a ^ t) - t;
}
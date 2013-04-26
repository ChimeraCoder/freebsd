
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

/* Returns: the index of the least significant 1-bit in a, or
 * the value zero if a is zero. The least significant bit is index one.
 */

si_int
__ffsti2(ti_int a)
{
    twords x;
    x.all = a;
    if (x.s.low == 0)
    {
        if (x.s.high == 0)
            return 0;
        return __builtin_ctzll(x.s.high) + (1 + sizeof(di_int) * CHAR_BIT);
    }
    return __builtin_ctzll(x.s.low) + 1;
}

#endif /* __x86_64 */
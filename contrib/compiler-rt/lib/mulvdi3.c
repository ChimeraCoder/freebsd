
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

/* Returns: a * b */

/* Effects: aborts if a * b overflows */

di_int
__mulvdi3(di_int a, di_int b)
{
    const int N = (int)(sizeof(di_int) * CHAR_BIT);
    const di_int MIN = (di_int)1 << (N-1);
    const di_int MAX = ~MIN;
    if (a == MIN)
    {
        if (b == 0 || b == 1)
            return a * b;
        compilerrt_abort();
    }
    if (b == MIN)
    {
        if (a == 0 || a == 1)
            return a * b;
        compilerrt_abort();
    }
    di_int sa = a >> (N - 1);
    di_int abs_a = (a ^ sa) - sa;
    di_int sb = b >> (N - 1);
    di_int abs_b = (b ^ sb) - sb;
    if (abs_a < 2 || abs_b < 2)
        return a * b;
    if (sa == sb)
    {
        if (abs_a > MAX / abs_b)
            compilerrt_abort();
    }
    else
    {
        if (abs_a > MIN / -abs_b)
            compilerrt_abort();
    }
    return a * b;
}
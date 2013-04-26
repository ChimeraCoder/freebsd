
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

 * This file implements __multi3 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

#if __x86_64

/* Returns: a * b */

static
ti_int
__mulddi3(du_int a, du_int b)
{
    twords r;
    const int bits_in_dword_2 = (int)(sizeof(di_int) * CHAR_BIT) / 2;
    const du_int lower_mask = (du_int)~0 >> bits_in_dword_2;
    r.s.low = (a & lower_mask) * (b & lower_mask);
    du_int t = r.s.low >> bits_in_dword_2;
    r.s.low &= lower_mask;
    t += (a >> bits_in_dword_2) * (b & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_dword_2;
    r.s.high = t >> bits_in_dword_2;
    t = r.s.low >> bits_in_dword_2;
    r.s.low &= lower_mask;
    t += (b >> bits_in_dword_2) * (a & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_dword_2;
    r.s.high += t >> bits_in_dword_2;
    r.s.high += (a >> bits_in_dword_2) * (b >> bits_in_dword_2);
    return r.all;
}

/* Returns: a * b */

ti_int
__multi3(ti_int a, ti_int b)
{
    twords x;
    x.all = a;
    twords y;
    y.all = b;
    twords r;
    r.all = __mulddi3(x.s.low, y.s.low);
    r.s.high += x.s.high * y.s.low + x.s.low * y.s.high;
    return r.all;
}

#endif /* __x86_64 */
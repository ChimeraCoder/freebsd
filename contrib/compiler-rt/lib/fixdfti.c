
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

/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: double is a IEEE 64 bit floating point type 
 *             su_int is a 32 bit integral type
 *             value in double is representable in ti_int (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ti_int
__fixdfti(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0)
        return 0;
    ti_int s = (si_int)(fb.u.s.high & 0x80000000) >> 31;
    ti_int r = 0x0010000000000000uLL | (0x000FFFFFFFFFFFFFuLL & fb.u.all);
    if (e > 52)
        r <<= (e - 52);
    else
        r >>= (52 - e);
    return (r ^ s) - s;
}

#endif

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

/* Returns: convert a to a unsigned long long, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: double is a IEEE 64 bit floating point type 
 *             tu_int is a 64 bit integral type
 *             value in double is representable in tu_int or is negative 
 *                 (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

tu_int
__fixunsdfti(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.s.high & 0x80000000))
        return 0;
    tu_int r = 0x0010000000000000uLL | (fb.u.all & 0x000FFFFFFFFFFFFFuLL);
    if (e > 52)
        r <<= (e - 52);
    else
        r >>= (52 - e);
    return r;
}

#endif
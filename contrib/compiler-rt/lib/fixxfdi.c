
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

#if !_ARCH_PPC

#include "int_lib.h"

/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: long double is an intel 80 bit floating point type padded with 6 bytes
 *             su_int is a 32 bit integral type
 *             value in long double is representable in di_int (no range checking performed)
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

di_int
__fixxfdi(long double a)
{
    long_double_bits fb;
    fb.f = a;
    int e = (fb.u.high.s.low & 0x00007FFF) - 16383;
    if (e < 0)
        return 0;
    di_int s = -(si_int)((fb.u.high.s.low & 0x00008000) >> 15);
    di_int r = fb.u.low.all;
    r = (du_int)r >> (63 - e);
    return (r ^ s) - s;
}

#endif /* !_ARCH_PPC */
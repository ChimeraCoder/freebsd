
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

/* Returns: convert a to a long double, rounding toward even. */

/* Assumption: long double is a IEEE 80 bit floating point type padded to 128 bits
 *             di_int is a 64 bit integral type
 */

/* gggg gggg gggg gggg gggg gggg gggg gggg | gggg gggg gggg gggg seee eeee eeee eeee |
 * 1mmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm
 */

long double
__floatdixf(di_int a)
{
    if (a == 0)
        return 0.0;
    const unsigned N = sizeof(di_int) * CHAR_BIT;
    const di_int s = a >> (N-1);
    a = (a ^ s) - s;
    int clz = __builtin_clzll(a);
    int e = (N - 1) - clz ;    /* exponent */
    long_double_bits fb;
    fb.u.high.s.low = ((su_int)s & 0x00008000) |  /* sign */
		      (e + 16383);                /* exponent */
    fb.u.low.all = a << clz;                    /* mantissa */
    return fb.f;
}

#endif /* !_ARCH_PPC */
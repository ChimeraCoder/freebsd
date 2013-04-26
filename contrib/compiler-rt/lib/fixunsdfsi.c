
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

/* Returns: convert a to a unsigned int, rounding toward zero.
 *          Negative values all become zero.
 */

/* Assumption: double is a IEEE 64 bit floating point type 
 *             su_int is a 32 bit integral type
 *             value in double is representable in su_int or is negative 
 *                 (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(d2uiz, fixunsdfsi)

COMPILER_RT_ABI su_int
__fixunsdfsi(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0 || (fb.u.s.high & 0x80000000))
        return 0;
    return (
                0x80000000u                      |
                ((fb.u.s.high & 0x000FFFFF) << 11) |
                (fb.u.s.low >> 21)
           ) >> (31 - e);
}
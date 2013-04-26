
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

/* Assumption: float is a IEEE 32 bit floating point type 
 *             su_int is a 32 bit integral type
 *             value in float is representable in su_int or is negative 
 *                 (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(f2uiz, fixunssfsi)

COMPILER_RT_ABI su_int
__fixunssfsi(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0 || (fb.u & 0x80000000))
        return 0;
    su_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return r;
}
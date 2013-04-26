
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

/* Returns: convert a to a signed long long, rounding toward zero. */

/* Assumption: float is a IEEE 32 bit floating point type 
 *             su_int is a 32 bit integral type
 *             value in float is representable in di_int (no range checking performed)
 */

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(f2lz, fixsfdi)

COMPILER_RT_ABI di_int
__fixsfdi(float a)
{
    float_bits fb;
    fb.f = a;
    int e = ((fb.u & 0x7F800000) >> 23) - 127;
    if (e < 0)
        return 0;
    di_int s = (si_int)(fb.u & 0x80000000) >> 31;
    di_int r = (fb.u & 0x007FFFFF) | 0x00800000;
    if (e > 23)
        r <<= (e - 23);
    else
        r >>= (23 - e);
    return (r ^ s) - s;
}
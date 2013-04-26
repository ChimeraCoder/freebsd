
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

/* Assumption: double is a IEEE 64 bit floating point type 
 *            su_int is a 32 bit integral type
 *            value in double is representable in di_int (no range checking performed)
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */

ARM_EABI_FNALIAS(d2lz, fixdfdi)

di_int
__fixdfdi(double a)
{
    double_bits fb;
    fb.f = a;
    int e = ((fb.u.s.high & 0x7FF00000) >> 20) - 1023;
    if (e < 0)
        return 0;
    di_int s = (si_int)(fb.u.s.high & 0x80000000) >> 31;
    dwords r;
    r.s.high = (fb.u.s.high & 0x000FFFFF) | 0x00100000;
    r.s.low = fb.u.s.low;
    if (e > 52)
        r.all <<= (e - 52);
    else
        r.all >>= (52 - e);
    return (r.all ^ s) - s;
} 
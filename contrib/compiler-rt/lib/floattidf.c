
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

/* Returns: convert a to a double, rounding toward even.*/

/* Assumption: double is a IEEE 64 bit floating point type 
 *            ti_int is a 128 bit integral type
 */

/* seee eeee eeee mmmm mmmm mmmm mmmm mmmm | mmmm mmmm mmmm mmmm mmmm mmmm mmmm mmmm */ 

si_int __clzti2(ti_int a);

double
__floattidf(ti_int a)
{
    if (a == 0)
        return 0.0;
    const unsigned N = sizeof(ti_int) * CHAR_BIT;
    const ti_int s = a >> (N-1);
    a = (a ^ s) - s;
    int sd = N - __clzti2(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > DBL_MANT_DIG)
    {
        /* start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR
         *                                               12345678901234567890123456
         * 1 = msb 1 bit
         * P = bit DBL_MANT_DIG-1 bits to the right of 1
         * Q = bit DBL_MANT_DIG bits to the right of 1
         * R = "or" of all bits to the right of Q
         */
        switch (sd)
        {
        case DBL_MANT_DIG + 1:
            a <<= 1;
            break;
        case DBL_MANT_DIG + 2:
            break;
        default:
            a = ((tu_int)a >> (sd - (DBL_MANT_DIG+2))) |
                ((a & ((tu_int)(-1) >> ((N + DBL_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to DBL_MANT_DIG or DBL_MANT_DIG+1 bits */
        if (a & ((tu_int)1 << DBL_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to DBL_MANT_DIG bits */
    }
    else
    {
        a <<= (DBL_MANT_DIG - sd);
        /* a is now rounded to DBL_MANT_DIG bits */
    }
    double_bits fb;
    fb.u.s.high = ((su_int)s & 0x80000000) |        /* sign */
                ((e + 1023) << 20)      |        /* exponent */
                ((su_int)(a >> 32) & 0x000FFFFF); /* mantissa-high */
    fb.u.s.low = (su_int)a;                         /* mantissa-low */
    return fb.f;
}

#endif
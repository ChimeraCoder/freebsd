
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

/* Returns: convert a to a float, rounding toward even.*/

/* Assumption: float is a IEEE 32 bit floating point type 
 *             di_int is a 64 bit integral type
 */ 

/* seee eeee emmm mmmm mmmm mmmm mmmm mmmm */

#include "int_lib.h"

ARM_EABI_FNALIAS(l2f, floatdisf)

COMPILER_RT_ABI float
__floatdisf(di_int a)
{
    if (a == 0)
        return 0.0F;
    const unsigned N = sizeof(di_int) * CHAR_BIT;
    const di_int s = a >> (N-1);
    a = (a ^ s) - s;
    int sd = N - __builtin_clzll(a);  /* number of significant digits */
    int e = sd - 1;             /* exponent */
    if (sd > FLT_MANT_DIG)
    {
        /*  start:  0000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQxxxxxxxxxxxxxxxxxx 
         *  finish: 000000000000000000000000000000000000001xxxxxxxxxxxxxxxxxxxxxxPQR 
         *                                                12345678901234567890123456 
         *  1 = msb 1 bit 
         *  P = bit FLT_MANT_DIG-1 bits to the right of 1 
         *  Q = bit FLT_MANT_DIG bits to the right of 1   
         *  R = "or" of all bits to the right of Q 
         */
        switch (sd)
        {
        case FLT_MANT_DIG + 1:
            a <<= 1;
            break;
        case FLT_MANT_DIG + 2:
            break;
        default:
            a = ((du_int)a >> (sd - (FLT_MANT_DIG+2))) |
                ((a & ((du_int)(-1) >> ((N + FLT_MANT_DIG+2) - sd))) != 0);
        };
        /* finish: */
        a |= (a & 4) != 0;  /* Or P into R */
        ++a;  /* round - this step may add a significant bit */
        a >>= 2;  /* dump Q and R */
        /* a is now rounded to FLT_MANT_DIG or FLT_MANT_DIG+1 bits */
        if (a & ((du_int)1 << FLT_MANT_DIG))
        {
            a >>= 1;
            ++e;
        }
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    else
    {
        a <<= (FLT_MANT_DIG - sd);
        /* a is now rounded to FLT_MANT_DIG bits */
    }
    float_bits fb;
    fb.u = ((su_int)s & 0x80000000) |  /* sign */
           ((e + 127) << 23)       |  /* exponent */
           ((su_int)a & 0x007FFFFF);   /* mantissa */
    return fb.f;
}

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

/* Returns: a / b */

/* Translated from Figure 3-40 of The PowerPC Compiler Writer's Guide */

ARM_EABI_FNALIAS(uidiv, udivsi3)

/* This function should not call __divsi3! */
COMPILER_RT_ABI su_int
__udivsi3(su_int n, su_int d)
{
    const unsigned n_uword_bits = sizeof(su_int) * CHAR_BIT;
    su_int q;
    su_int r;
    unsigned sr;
    /* special cases */
    if (d == 0)
        return 0; /* ?! */
    if (n == 0)
        return 0;
    sr = __builtin_clz(d) - __builtin_clz(n);
    /* 0 <= sr <= n_uword_bits - 1 or sr large */
    if (sr > n_uword_bits - 1)  /* d > r */
        return 0;
    if (sr == n_uword_bits - 1)  /* d == 1 */
        return n;
    ++sr;
    /* 1 <= sr <= n_uword_bits - 1 */
    /* Not a special case */
    q = n << (n_uword_bits - sr);
    r = n >> sr;
    su_int carry = 0;
    for (; sr > 0; --sr)
    {
        /* r:q = ((r:q)  << 1) | carry */
        r = (r << 1) | (q >> (n_uword_bits - 1));
        q = (q << 1) | carry;
        /* carry = 0;
         * if (r.all >= d.all)
         * {
         *      r.all -= d.all;
         *      carry = 1;
         * }
         */
        const si_int s = (si_int)(d - r - 1) >> (n_uword_bits - 1);
        carry = s & 1;
        r -= d & s;
    }
    q = (q << 1) | carry;
    return q;
}
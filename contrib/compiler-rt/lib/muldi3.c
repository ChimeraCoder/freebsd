
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

/* Returns: a * b */

static
di_int
__muldsi3(su_int a, su_int b)
{
    dwords r;
    const int bits_in_word_2 = (int)(sizeof(si_int) * CHAR_BIT) / 2;
    const su_int lower_mask = (su_int)~0 >> bits_in_word_2;
    r.s.low = (a & lower_mask) * (b & lower_mask);
    su_int t = r.s.low >> bits_in_word_2;
    r.s.low &= lower_mask;
    t += (a >> bits_in_word_2) * (b & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_word_2;
    r.s.high = t >> bits_in_word_2;
    t = r.s.low >> bits_in_word_2;
    r.s.low &= lower_mask;
    t += (b >> bits_in_word_2) * (a & lower_mask);
    r.s.low += (t & lower_mask) << bits_in_word_2;
    r.s.high += t >> bits_in_word_2;
    r.s.high += (a >> bits_in_word_2) * (b >> bits_in_word_2);
    return r.all;
}

/* Returns: a * b */

ARM_EABI_FNALIAS(lmul, muldi3)

COMPILER_RT_ABI di_int
__muldi3(di_int a, di_int b)
{
    dwords x;
    x.all = a;
    dwords y;
    y.all = b;
    dwords r;
    r.all = __muldsi3(x.s.low, y.s.low);
    r.s.high += x.s.high * y.s.low + x.s.low * y.s.high;
    return r.all;
}
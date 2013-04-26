
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

su_int COMPILER_RT_ABI __udivsi3(su_int n, su_int d);

/* Returns: a / b */

ARM_EABI_FNALIAS(idiv, divsi3)

COMPILER_RT_ABI si_int
__divsi3(si_int a, si_int b)
{
    const int bits_in_word_m1 = (int)(sizeof(si_int) * CHAR_BIT) - 1;
    si_int s_a = a >> bits_in_word_m1;           /* s_a = a < 0 ? -1 : 0 */
    si_int s_b = b >> bits_in_word_m1;           /* s_b = b < 0 ? -1 : 0 */
    a = (a ^ s_a) - s_a;                         /* negate if s_a == -1 */
    b = (b ^ s_b) - s_b;                         /* negate if s_b == -1 */
    s_a ^= s_b;                                  /* sign of quotient */
    /*
     * On CPUs without unsigned hardware division support,
     *  this calls __udivsi3 (notice the cast to su_int).
     * On CPUs with unsigned hardware division support,
     *  this uses the unsigned division instruction.
     */
    return ((su_int)a/(su_int)b ^ s_a) - s_a;    /* negate if s_a == -1 */
}
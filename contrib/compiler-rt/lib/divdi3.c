
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

du_int COMPILER_RT_ABI __udivmoddi4(du_int a, du_int b, du_int* rem);

/* Returns: a / b */

COMPILER_RT_ABI di_int
__divdi3(di_int a, di_int b)
{
    const int bits_in_dword_m1 = (int)(sizeof(di_int) * CHAR_BIT) - 1;
    di_int s_a = a >> bits_in_dword_m1;           /* s_a = a < 0 ? -1 : 0 */
    di_int s_b = b >> bits_in_dword_m1;           /* s_b = b < 0 ? -1 : 0 */
    a = (a ^ s_a) - s_a;                         /* negate if s_a == -1 */
    b = (b ^ s_b) - s_b;                         /* negate if s_b == -1 */
    s_a ^= s_b;                                  /*sign of quotient */
    return (__udivmoddi4(a, b, (du_int*)0) ^ s_a) - s_a;  /* negate if s_a == -1 */
}
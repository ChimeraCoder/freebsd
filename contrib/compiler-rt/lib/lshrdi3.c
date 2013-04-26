
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

/* Returns: logical a >> b */

/* Precondition:  0 <= b < bits_in_dword */

ARM_EABI_FNALIAS(llsr, lshrdi3)

COMPILER_RT_ABI di_int
__lshrdi3(di_int a, si_int b)
{
    const int bits_in_word = (int)(sizeof(si_int) * CHAR_BIT);
    udwords input;
    udwords result;
    input.all = a;
    if (b & bits_in_word)  /* bits_in_word <= b < bits_in_dword */
    {
        result.s.high = 0;
        result.s.low = input.s.high >> (b - bits_in_word);
    }
    else  /* 0 <= b < bits_in_word */
    {
        if (b == 0)
            return a;
        result.s.high  = input.s.high >> b;
        result.s.low = (input.s.high << (bits_in_word - b)) | (input.s.low >> b);
    }
    return result.all;
}
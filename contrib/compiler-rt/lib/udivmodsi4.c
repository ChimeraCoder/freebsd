
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

extern su_int COMPILER_RT_ABI __udivsi3(su_int n, su_int d);


/* Returns: a / b, *rem = a % b  */

COMPILER_RT_ABI su_int
__udivmodsi4(su_int a, su_int b, su_int* rem)
{
  si_int d = __udivsi3(a,b);
  *rem = a - (d*b);
  return d;
}
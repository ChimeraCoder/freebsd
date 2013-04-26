
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

extern COMPILER_RT_ABI di_int __divdi3(di_int a, di_int b);

/* Returns: a / b, *rem = a % b  */

COMPILER_RT_ABI di_int
__divmoddi4(di_int a, di_int b, di_int* rem)
{
  di_int d = __divdi3(a,b);
  *rem = a - (d*b);
  return d;
}
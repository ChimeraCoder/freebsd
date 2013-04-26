
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

/* Returns: a % b */

COMPILER_RT_ABI du_int
__umoddi3(du_int a, du_int b)
{
    du_int r;
    __udivmoddi4(a, b, &r);
    return r;
}
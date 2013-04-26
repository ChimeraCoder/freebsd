
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

/* Returns: -a */

/* Effects: aborts if -a overflows */

COMPILER_RT_ABI di_int
__negvdi2(di_int a)
{
    const di_int MIN = (di_int)1 << ((int)(sizeof(di_int) * CHAR_BIT)-1);
    if (a == MIN)
        compilerrt_abort();
    return -a;
}
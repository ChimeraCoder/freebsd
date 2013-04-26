
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

#if __x86_64

/* Returns: -a */

/* Effects: aborts if -a overflows */

ti_int
__negvti2(ti_int a)
{
    const ti_int MIN = (ti_int)1 << ((int)(sizeof(ti_int) * CHAR_BIT)-1);
    if (a == MIN)
        compilerrt_abort();
    return -a;
}

#endif
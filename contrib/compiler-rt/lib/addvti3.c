
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

/* Returns: a + b */

/* Effects: aborts if a + b overflows */

ti_int
__addvti3(ti_int a, ti_int b)
{
    ti_int s = a + b;
    if (b >= 0)
    {
        if (s < a)
            compilerrt_abort();
    }
    else
    {
        if (s >= a)
            compilerrt_abort();
    }
    return s;
}

#endif

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

/* Returns: the number of leading 0-bits */

/* Precondition: a != 0 */

si_int
__clzti2(ti_int a)
{
    twords x;
    x.all = a;
    const di_int f = -(x.s.high == 0);
    return __builtin_clzll((x.s.high & ~f) | (x.s.low & f)) +
           ((si_int)f & ((si_int)(sizeof(di_int) * CHAR_BIT)));
}

#endif /* __x86_64 */
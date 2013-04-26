
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

#include <config.h>
#include "roken.h"

/**
 * Constant time compare to memory regions. The reason for making it
 * constant time is to make sure that timeing information leak from
 * where in the function the diffrence is.
 *
 * ct_memcmp() can't be used to order memory regions like memcmp(),
 * for example, use ct_memcmp() with qsort().
 *
 * @param p1 memory region 1 to compare
 * @param p2 memory region 2 to compare
 * @param len length of memory
 *
 * @return 0 when the memory regions are equal, non zero if not
 *
 * @ingroup roken
 */

int
ct_memcmp(const void *p1, const void *p2, size_t len)
{
    const unsigned char *s1 = p1, *s2 = p2;
    size_t i;
    int r = 0;

    for (i = 0; i < len; i++)
	r |= (s1[i] ^ s2[i]);
    return !!r;
}
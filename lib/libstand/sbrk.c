
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Minimal sbrk() emulation required for malloc support.
 */

#include <string.h>
#include "stand.h"

static size_t	maxheap, heapsize = 0;
static void	*heapbase;

void
setheap(void *base, void *top)
{
    /* Align start address to 16 bytes for the malloc code. Sigh. */
    heapbase = (void *)(((uintptr_t)base + 15) & ~15);
    maxheap = (char *)top - (char *)heapbase;
}

char *
sbrk(int incr)
{
    char	*ret;
    
    if ((heapsize + incr) <= maxheap) {
	ret = (char *)heapbase + heapsize;
	bzero(ret, incr);
	heapsize += incr;
	return(ret);
    }
    errno = ENOMEM;
    return((char *)-1);
}
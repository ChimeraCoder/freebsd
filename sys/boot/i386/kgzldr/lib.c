
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

#include <sys/types.h>
#include <stddef.h>

#include "kgzldr.h"

#define MEMSIZ  0x8000		/* Memory pool size */

int kgz_con;			/* Console control */

static size_t memtot;		/* Memory allocated: bytes */
static u_int memcnt;		/* Memory allocated: blocks */

/*
 * Library functions required by inflate().
 */

/*
 * Allocate memory block.
 */
unsigned char *
kzipmalloc(int size)
{
    static u_char mem[MEMSIZ];
    void *ptr;

    if (memtot + size > MEMSIZ)
        return NULL;
    ptr = mem + memtot;
    memtot += size;
    memcnt++;
    return ptr;
}

/*
 * Free allocated memory block.
 */
void
kzipfree(void *ptr)
{
    memcnt--;
    if (!memcnt)
        memtot = 0;
}

/*
 * Write a string to the console.
 */
void
putstr(const char *str)
{
    int c;

    while ((c = *str++)) {
        if (kgz_con & KGZ_CRT)
            crt_putchr(c);
        if (kgz_con & KGZ_SIO)
            sio_putchr(c);
    }
}
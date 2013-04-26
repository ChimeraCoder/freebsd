
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

#include "common.h"
#include <stdio.h>
#include <err.h>
#include "roken.h"

RCSID("$Id$");

/*
 * Allocate a buffer enough to handle st->st_blksize, if
 * there is such a field, otherwise BUFSIZ.
 */

void *
alloc_buffer (void *oldbuf, size_t *sz, struct stat *st)
{
    size_t new_sz;

    new_sz = BUFSIZ;
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
    if (st)
	new_sz = max(BUFSIZ, st->st_blksize);
#endif
    if(new_sz > *sz) {
	if (oldbuf)
	    free (oldbuf);
	oldbuf = malloc (new_sz);
	if (oldbuf == NULL) {
	    warn ("malloc");
	    *sz = 0;
	    return NULL;
	}
	*sz = new_sz;
    }
    return oldbuf;
}
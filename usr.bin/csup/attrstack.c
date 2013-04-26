
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

#include <assert.h>
#include <stdlib.h>

#include "attrstack.h"
#include "fattr.h"
#include "misc.h"

#define	ATTRSTACK_DEFSIZE	16	/* Initial size of the stack. */

struct attrstack {
	struct fattr **stack;
	size_t cur;
	size_t size;
};

struct attrstack *
attrstack_new(void)
{
	struct attrstack *as;

	as = xmalloc(sizeof(struct attrstack));
	as->stack = xmalloc(sizeof(struct fattr *) * ATTRSTACK_DEFSIZE);
	as->size = ATTRSTACK_DEFSIZE;
	as->cur = 0;
	return (as);
}

struct fattr *
attrstack_pop(struct attrstack *as)
{

	assert(as->cur > 0);
	return (as->stack[--as->cur]);
}

void
attrstack_push(struct attrstack *as, struct fattr *fa)
{

	if (as->cur >= as->size) {
		as->size *= 2;
		as->stack = xrealloc(as->stack,
		    sizeof(struct fattr *) * as->size);
	}
	as->stack[as->cur++] = fa;
}

size_t
attrstack_size(struct attrstack *as)
{

	return (as->cur);
}

void
attrstack_free(struct attrstack *as)
{

	assert(as->cur == 0);
	free(as->stack);
	free(as);
}
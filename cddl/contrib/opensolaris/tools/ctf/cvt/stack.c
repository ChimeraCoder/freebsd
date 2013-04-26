
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

/*
 * Routines for manipulating stacks
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "stack.h"
#include "memory.h"

#define	STACK_SEEDSIZE	5

struct stk {
	int st_nument;
	int st_top;
	void **st_data;

	void (*st_free)(void *);
};

stk_t *
stack_new(void (*freep)(void *))
{
	stk_t *sp;

	sp = xmalloc(sizeof (stk_t));
	sp->st_nument = STACK_SEEDSIZE;
	sp->st_top = -1;
	sp->st_data = xmalloc(sizeof (void *) * sp->st_nument);
	sp->st_free = freep;

	return (sp);
}

void
stack_free(stk_t *sp)
{
	int i;

	if (sp->st_free) {
		for (i = 0; i <= sp->st_top; i++)
			sp->st_free(sp->st_data[i]);
	}
	free(sp->st_data);
	free(sp);
}

void *
stack_pop(stk_t *sp)
{
	assert(sp->st_top >= 0);

	return (sp->st_data[sp->st_top--]);
}

void *
stack_peek(stk_t *sp)
{
	if (sp->st_top == -1)
		return (NULL);

	return (sp->st_data[sp->st_top]);
}

void
stack_push(stk_t *sp, void *data)
{
	sp->st_top++;

	if (sp->st_top == sp->st_nument) {
		sp->st_nument += STACK_SEEDSIZE;
		sp->st_data = xrealloc(sp->st_data,
		    sizeof (void *) * sp->st_nument);
	}

	sp->st_data[sp->st_top] = data;
}

int
stack_level(stk_t *sp)
{
	return (sp->st_top + 1);
}
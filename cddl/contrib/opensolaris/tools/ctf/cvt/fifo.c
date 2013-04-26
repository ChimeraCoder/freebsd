
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
 * Routines for manipulating a FIFO queue
 */

#include <stdlib.h>

#include "fifo.h"
#include "memory.h"

typedef struct fifonode {
	void *fn_data;
	struct fifonode *fn_next;
} fifonode_t;

struct fifo {
	fifonode_t *f_head;
	fifonode_t *f_tail;
};

fifo_t *
fifo_new(void)
{
	fifo_t *f;

	f = xcalloc(sizeof (fifo_t));

	return (f);
}

/* Add to the end of the fifo */
void
fifo_add(fifo_t *f, void *data)
{
	fifonode_t *fn = xmalloc(sizeof (fifonode_t));

	fn->fn_data = data;
	fn->fn_next = NULL;

	if (f->f_tail == NULL)
		f->f_head = f->f_tail = fn;
	else {
		f->f_tail->fn_next = fn;
		f->f_tail = fn;
	}
}

/* Remove from the front of the fifo */
void *
fifo_remove(fifo_t *f)
{
	fifonode_t *fn;
	void *data;

	if ((fn = f->f_head) == NULL)
		return (NULL);

	data = fn->fn_data;
	if ((f->f_head = fn->fn_next) == NULL)
		f->f_tail = NULL;

	free(fn);

	return (data);
}

/*ARGSUSED*/
static void
fifo_nullfree(void *arg)
{
	/* this function intentionally left blank */
}

/* Free an entire fifo */
void
fifo_free(fifo_t *f, void (*freefn)(void *))
{
	fifonode_t *fn = f->f_head;
	fifonode_t *tmp;

	if (freefn == NULL)
		freefn = fifo_nullfree;

	while (fn) {
		(*freefn)(fn->fn_data);

		tmp = fn;
		fn = fn->fn_next;
		free(tmp);
	}

	free(f);
}

int
fifo_len(fifo_t *f)
{
	fifonode_t *fn;
	int i;

	for (i = 0, fn = f->f_head; fn; fn = fn->fn_next, i++);

	return (i);
}

int
fifo_empty(fifo_t *f)
{
	return (f->f_head == NULL);
}

int
fifo_iter(fifo_t *f, int (*iter)(void *data, void *arg), void *arg)
{
	fifonode_t *fn;
	int rc;
	int ret = 0;

	for (fn = f->f_head; fn; fn = fn->fn_next) {
		if ((rc = iter(fn->fn_data, arg)) < 0)
			return (-1);
		ret += rc;
	}

	return (ret);
}
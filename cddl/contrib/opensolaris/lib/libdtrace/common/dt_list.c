
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
 * Simple doubly-linked list implementation.  This implementation assumes that
 * each list element contains an embedded dt_list_t (previous and next
 * pointers), which is typically the first member of the element struct.
 * An additional dt_list_t is used to store the head (dl_next) and tail
 * (dl_prev) pointers.  The current head and tail list elements have their
 * previous and next pointers set to NULL, respectively.
 */

#include <unistd.h>
#include <assert.h>
#include <dt_list.h>

void
dt_list_append(dt_list_t *dlp, void *new)
{
	dt_list_t *p = dlp->dl_prev;	/* p = tail list element */
	dt_list_t *q = new;		/* q = new list element */

	dlp->dl_prev = q;
	q->dl_prev = p;
	q->dl_next = NULL;

	if (p != NULL) {
		assert(p->dl_next == NULL);
		p->dl_next = q;
	} else {
		assert(dlp->dl_next == NULL);
		dlp->dl_next = q;
	}
}

void
dt_list_prepend(dt_list_t *dlp, void *new)
{
	dt_list_t *p = new;		/* p = new list element */
	dt_list_t *q = dlp->dl_next;	/* q = head list element */

	dlp->dl_next = p;
	p->dl_prev = NULL;
	p->dl_next = q;

	if (q != NULL) {
		assert(q->dl_prev == NULL);
		q->dl_prev = p;
	} else {
		assert(dlp->dl_prev == NULL);
		dlp->dl_prev = p;
	}
}

void
dt_list_insert(dt_list_t *dlp, void *after_me, void *new)
{
	dt_list_t *p = after_me;
	dt_list_t *q = new;

	if (p == NULL || p->dl_next == NULL) {
		dt_list_append(dlp, new);
		return;
	}

	q->dl_next = p->dl_next;
	q->dl_prev = p;
	p->dl_next = q;
	q->dl_next->dl_prev = q;
}

void
dt_list_delete(dt_list_t *dlp, void *existing)
{
	dt_list_t *p = existing;

	if (p->dl_prev != NULL)
		p->dl_prev->dl_next = p->dl_next;
	else
		dlp->dl_next = p->dl_next;

	if (p->dl_next != NULL)
		p->dl_next->dl_prev = p->dl_prev;
	else
		dlp->dl_prev = p->dl_prev;
}
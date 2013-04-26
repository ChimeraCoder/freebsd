
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

#define	_SEARCH_PRIVATE
#include <search.h>
#ifdef DEBUG
#include <stdio.h>
#else
#include <stdlib.h>	/* for NULL */
#endif

void
insque(void *element, void *pred)
{
	struct que_elem *prev, *next, *elem;

	elem = (struct que_elem *)element;
	prev = (struct que_elem *)pred;

	if (prev == NULL) {
		elem->prev = elem->next = NULL;
		return;
	}

	next = prev->next;
	if (next != NULL) {
#ifdef DEBUG
		if (next->prev != prev) {
			fprintf(stderr, "insque: Inconsistency detected:"
			    " next(%p)->prev(%p) != prev(%p)\n",
			    next, next->prev, prev);
		}
#endif
		next->prev = elem;
	}
	prev->next = elem;
	elem->prev = prev;
	elem->next = next;
}
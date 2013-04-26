
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

#include "pqueue.h"

int
main(void)
	{
	pitem *item;
	pqueue pq;

	pq = pqueue_new();

	item = pitem_new(3, NULL);
	pqueue_insert(pq, item);

	item = pitem_new(1, NULL);
	pqueue_insert(pq, item);

	item = pitem_new(2, NULL);
	pqueue_insert(pq, item);

	item = pqueue_find(pq, 1);
	fprintf(stderr, "found %ld\n", item->priority);

	item = pqueue_find(pq, 2);
	fprintf(stderr, "found %ld\n", item->priority);

	item = pqueue_find(pq, 3);
	fprintf(stderr, "found %ld\n", item ? item->priority: 0);

	pqueue_print(pq);

	for(item = pqueue_pop(pq); item != NULL; item = pqueue_pop(pq))
		pitem_free(item);

	pqueue_free(pq);
	return 0;
	}
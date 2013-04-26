
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

/**
 * \file
 *
 * This file contains functions to help a module keep track of the
 * queries it has outstanding to authoritative servers.
 */
#include "config.h"
#include <sys/time.h>
#include "services/outbound_list.h"
#include "services/outside_network.h"

void 
outbound_list_init(struct outbound_list* list)
{
	list->first = NULL;
}

void 
outbound_list_clear(struct outbound_list* list)
{
	struct outbound_entry *p, *np;
	p = list->first;
	while(p) {
		np = p->next;
		outnet_serviced_query_stop(p->qsent, p);
		/* in region, no free needed */
		p = np;
	}
	outbound_list_init(list);
}

void 
outbound_list_insert(struct outbound_list* list, struct outbound_entry* e)
{
	if(list->first)
		list->first->prev = e;
	e->next = list->first;
	e->prev = NULL;
	list->first = e;
}

void 
outbound_list_remove(struct outbound_list* list, struct outbound_entry* e)
{
	if(!e)
		return;
	outnet_serviced_query_stop(e->qsent, e);
	if(e->next)
		e->next->prev = e->prev;
	if(e->prev)
		e->prev->next = e->next;
	else	list->first = e->next;
	/* in region, no free needed */
}
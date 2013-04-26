
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "agent.h"
#include "debug.h"

static int
agent_cmp_func(const void *a1, const void *a2)
{
   	struct agent const *ap1 = *((struct agent const **)a1);
	struct agent const *ap2 = *((struct agent const **)a2);
	int res;

	res = strcmp(ap1->name, ap2->name);
	if (res == 0) {
		if (ap1->type == ap2->type)
			res = 0;
		else if (ap1->type < ap2->type)
			res = -1;
		else
			res = 1;
	}

	return (res);
}

struct agent_table *
init_agent_table(void)
{
   	struct agent_table	*retval;

	TRACE_IN(init_agent_table);
	retval = calloc(1, sizeof(*retval));
	assert(retval != NULL);

	TRACE_OUT(init_agent_table);
	return (retval);
}

void
register_agent(struct agent_table *at, struct agent *a)
{
	struct agent **new_agents;
    	size_t new_agents_num;

	TRACE_IN(register_agent);
	assert(at != NULL);
	assert(a != NULL);
	new_agents_num = at->agents_num + 1;
	new_agents = malloc(sizeof(*new_agents) *
		new_agents_num);
	assert(new_agents != NULL);
	memcpy(new_agents, at->agents, at->agents_num * sizeof(struct agent *));
	new_agents[new_agents_num - 1] = a;
	qsort(new_agents, new_agents_num, sizeof(struct agent *),
		agent_cmp_func);

	free(at->agents);
	at->agents = new_agents;
	at->agents_num = new_agents_num;
    	TRACE_OUT(register_agent);
}

struct agent *
find_agent(struct agent_table *at, const char *name, enum agent_type type)
{
	struct agent **res;
	struct agent model, *model_p;

	TRACE_IN(find_agent);
	model.name = (char *)name;
	model.type = type;
	model_p = &model;
	res = bsearch(&model_p, at->agents, at->agents_num,
		sizeof(struct agent *), agent_cmp_func);

	TRACE_OUT(find_agent);
	return ( res == NULL ? NULL : *res);
}

void
destroy_agent_table(struct agent_table *at)
{
    	size_t i;

	TRACE_IN(destroy_agent_table);
	assert(at != NULL);
	for (i = 0; i < at->agents_num; ++i) {
		free(at->agents[i]->name);
		free(at->agents[i]);
	}

	free(at->agents);
	free(at);
	TRACE_OUT(destroy_agent_table);
}
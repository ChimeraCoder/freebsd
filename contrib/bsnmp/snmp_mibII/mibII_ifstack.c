
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
#include "mibII.h"

int
mib_ifstack_create(const struct mibif *lower, const struct mibif *upper)
{
	struct mibifstack *stack;

	if ((stack = malloc(sizeof(*stack))) == NULL)
		return (-1);

	stack->index.len = 2;
	stack->index.subs[0] = upper ? upper->index : 0;
	stack->index.subs[1] = lower ? lower->index : 0;

	INSERT_OBJECT_OID(stack, &mibifstack_list);

	mib_ifstack_last_change = get_ticks();

	return (0);
}

void
mib_ifstack_delete(const struct mibif *lower, const struct mibif *upper)
{
	struct mibifstack *stack;

	TAILQ_FOREACH(stack, &mibifstack_list, link)
		if (stack->index.subs[0] == (upper ? upper->index : 0) &&
		    stack->index.subs[1] == (lower ? lower->index : 0)) {
			TAILQ_REMOVE(&mibifstack_list, stack, link);
			free(stack);
			mib_ifstack_last_change = get_ticks();
			return;
		}
}

int
op_ifstack(struct snmp_context *ctx __unused, struct snmp_value *value,
    u_int sub, u_int iidx __unused, enum snmp_op op)
{
	struct mibifstack *stack;

	switch (op) {

	  case SNMP_OP_GETNEXT:
		if ((stack = NEXT_OBJECT_OID(&mibifstack_list, &value->var, sub)) == NULL)
			return (SNMP_ERR_NOSUCHNAME);
		index_append(&value->var, sub, &stack->index);
		break;

	  case SNMP_OP_GET:
		if ((stack = FIND_OBJECT_OID(&mibifstack_list, &value->var, sub)) == NULL)
			return (SNMP_ERR_NOSUCHNAME);
		break;

	  case SNMP_OP_SET:
		if ((stack = FIND_OBJECT_OID(&mibifstack_list, &value->var, sub)) == NULL)
			return (SNMP_ERR_NO_CREATION);
		return (SNMP_ERR_NOT_WRITEABLE);

	  case SNMP_OP_ROLLBACK:
	  case SNMP_OP_COMMIT:
		abort();
	}

	switch (value->var.subs[sub - 1]) {

	  case LEAF_ifStackStatus:
		value->v.integer = 1;
		break;
	}
	return (SNMP_ERR_NOERROR);
}
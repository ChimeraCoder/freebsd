
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
#include "mibII_oid.h"

/*
 * Scalars
 */
int
op_ifmib(struct snmp_context *ctx __unused, struct snmp_value *value,
    u_int sub, u_int idx __unused, enum snmp_op op)
{
	switch (op) {

	  case SNMP_OP_GETNEXT:
		abort();

	  case SNMP_OP_GET:
		break;

	  case SNMP_OP_SET:
		return (SNMP_ERR_NOT_WRITEABLE);

	  case SNMP_OP_ROLLBACK:
	  case SNMP_OP_COMMIT:
		abort();
	}

	switch (value->var.subs[sub - 1]) {

	  case LEAF_ifTableLastChange:
		if (mib_iftable_last_change > start_tick)
			value->v.uint32 = mib_iftable_last_change - start_tick;
		else
			value->v.uint32 = 0;
		break;

	  case LEAF_ifStackLastChange:
		if (mib_ifstack_last_change > start_tick)
			value->v.uint32 = mib_ifstack_last_change - start_tick;
		else
			value->v.uint32 = 0;
		break;
	}
	return (SNMP_ERR_NOERROR);
}
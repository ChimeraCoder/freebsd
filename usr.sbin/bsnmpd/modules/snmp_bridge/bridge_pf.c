
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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/socket.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_mib.h>
#include <net/if_types.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include <bsnmp/snmpmod.h>
#include <bsnmp/snmp_mibII.h>

#include "bridge_tree.h"
#include "bridge_snmp.h"

static int
val2snmp_truth(uint8_t val)
{
	if (val == 0)
		return (2);

	return (1);
}

static int
snmp_truth2val(int32_t truth)
{
	if (truth == 2)
		return (0);
	else if (truth == 1)
		return (1);

	return (-1);
}

int
op_begemot_bridge_pf(struct snmp_context *ctx, struct snmp_value *val,
	uint sub, uint iidx __unused, enum snmp_op op)
{
	int k_val;

	if (val->var.subs[sub - 1] > LEAF_begemotBridgeLayer2PfStatus)
		return (SNMP_ERR_NOSUCHNAME);

	switch (op) {
		case SNMP_OP_GETNEXT:
			abort();
		case SNMP_OP_ROLLBACK:
			bridge_do_pfctl(val->var.subs[sub - 1] - 1,
			    op, &(ctx->scratch->int1));
				return (SNMP_ERR_NOERROR);

		case SNMP_OP_COMMIT:
			return (SNMP_ERR_NOERROR);

		case SNMP_OP_SET:
			ctx->scratch->int1 =
			    bridge_get_pfval(val->var.subs[sub - 1]);

			if ((k_val = snmp_truth2val(val->v.integer)) < 0)
				return (SNMP_ERR_BADVALUE);
			return (SNMP_ERR_NOERROR);

		case SNMP_OP_GET:
			switch (val->var.subs[sub - 1]) {
			    case LEAF_begemotBridgePfilStatus:
			    case LEAF_begemotBridgePfilMembers:
			    case LEAF_begemotBridgePfilIpOnly:
			    case LEAF_begemotBridgeLayer2PfStatus:
				if (bridge_do_pfctl(val->var.subs[sub - 1] - 1,
				    op, &k_val) < 0)
					return (SNMP_ERR_GENERR);
				val->v.integer = val2snmp_truth(k_val);
				return (SNMP_ERR_NOERROR);
			}
			abort();
	}

	abort();
}
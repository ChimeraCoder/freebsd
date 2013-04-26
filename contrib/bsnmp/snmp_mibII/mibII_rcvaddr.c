
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
 * find receive address
 */
struct mibrcvaddr *
mib_find_rcvaddr(u_int ifindex, const u_char *addr, size_t addrlen)
{
	struct mibrcvaddr *rcv;

	TAILQ_FOREACH(rcv, &mibrcvaddr_list, link)
		if (rcv->ifindex == ifindex &&
		    rcv->addrlen == addrlen &&
		    memcmp(rcv->addr, addr, addrlen) == 0)
			return (rcv);
	return (NULL);
}

/*
 * Create receive address
 */
struct mibrcvaddr *
mib_rcvaddr_create(struct mibif *ifp, const u_char *addr, size_t addrlen)
{
	struct mibrcvaddr *rcv;
	u_int i;

	if (addrlen + OIDLEN_ifRcvAddressEntry + 1 > ASN_MAXOIDLEN)
		return (NULL);

	if ((rcv = malloc(sizeof(*rcv))) == NULL)
		return (NULL);
	rcv->ifindex = ifp->index;
	rcv->addrlen = addrlen;
	memcpy(rcv->addr, addr, addrlen);
	rcv->flags = 0;

	rcv->index.len = addrlen + 2;
	rcv->index.subs[0] = ifp->index;
	rcv->index.subs[1] = addrlen;
	for (i = 0; i < addrlen; i++)
		rcv->index.subs[i + 2] = addr[i];

	INSERT_OBJECT_OID(rcv, &mibrcvaddr_list);

	return (rcv);
}

/*
 * Delete a receive address
 */
void
mib_rcvaddr_delete(struct mibrcvaddr *rcv)
{
	TAILQ_REMOVE(&mibrcvaddr_list, rcv, link);
	free(rcv);
}

int
op_rcvaddr(struct snmp_context *ctx __unused, struct snmp_value *value,
    u_int sub, u_int iidx __unused, enum snmp_op op)
{
	struct mibrcvaddr *rcv;

	rcv = NULL;	/* make compiler happy */

	switch (op) {

	  case SNMP_OP_GETNEXT:
		if ((rcv = NEXT_OBJECT_OID(&mibrcvaddr_list, &value->var, sub)) == NULL)
			return (SNMP_ERR_NOSUCHNAME);
		index_append(&value->var, sub, &rcv->index);
		break;

	  case SNMP_OP_GET:
		if ((rcv = FIND_OBJECT_OID(&mibrcvaddr_list, &value->var, sub)) == NULL)
			return (SNMP_ERR_NOSUCHNAME);
		break;

	  case SNMP_OP_SET:
		if ((rcv = FIND_OBJECT_OID(&mibrcvaddr_list, &value->var, sub)) == NULL)
			return (SNMP_ERR_NO_CREATION);
		return (SNMP_ERR_NOT_WRITEABLE);

	  case SNMP_OP_ROLLBACK:
	  case SNMP_OP_COMMIT:
		abort();
	}

	switch (value->var.subs[sub - 1]) {

	  case LEAF_ifRcvAddressStatus:
		value->v.integer = 1;
		break;

	  case LEAF_ifRcvAddressType:
		value->v.integer = (rcv->flags & MIBRCVADDR_VOLATILE) ? 2 : 3;
		break;
	}
	return (SNMP_ERR_NOERROR);
}

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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include <mad.h>
#include <infiniband/common.h>

#undef DEBUG
#define DEBUG 	if (ibdebug)	IBWARN

uint8_t *
smp_set_via(void *data, ib_portid_t *portid, unsigned attrid, unsigned mod, unsigned timeout, const void *srcport)
{
	ib_rpc_t rpc = {0};

	DEBUG("attr 0x%x mod 0x%x route %s", attrid, mod, portid2str(portid));
	if ((portid->lid <= 0) ||
	    (portid->drpath.drslid == 0xffff) ||
	    (portid->drpath.drdlid == 0xffff))
		rpc.mgtclass = IB_SMI_DIRECT_CLASS;	/* direct SMI */
	else
		rpc.mgtclass = IB_SMI_CLASS;		/* Lid routed SMI */

	rpc.method = IB_MAD_METHOD_SET;
	rpc.attr.id = attrid;
	rpc.attr.mod = mod;
	rpc.timeout = timeout;
	rpc.datasz = IB_SMP_DATA_SIZE;
	rpc.dataoffs = IB_SMP_DATA_OFFS;

	portid->sl = 0;
	portid->qp = 0;

	if (srcport) {
		return mad_rpc(srcport, &rpc, portid, data, data);
	} else {
		return madrpc(&rpc, portid, data, data);
	}
}

uint8_t *
smp_set(void *data, ib_portid_t *portid, unsigned attrid, unsigned mod, unsigned timeout)
{
	return smp_set_via(data, portid, attrid, mod, timeout, NULL);
}

uint8_t *
smp_query_via(void *rcvbuf, ib_portid_t *portid, unsigned attrid, unsigned mod,
	      unsigned timeout, const void *srcport)
{
	ib_rpc_t rpc = {0};

	DEBUG("attr 0x%x mod 0x%x route %s", attrid, mod, portid2str(portid));
	rpc.method = IB_MAD_METHOD_GET;
	rpc.attr.id = attrid;
	rpc.attr.mod = mod;
	rpc.timeout = timeout;
	rpc.datasz = IB_SMP_DATA_SIZE;
	rpc.dataoffs = IB_SMP_DATA_OFFS;

	if ((portid->lid <= 0) ||
	    (portid->drpath.drslid == 0xffff) ||
	    (portid->drpath.drdlid == 0xffff))
		rpc.mgtclass = IB_SMI_DIRECT_CLASS;	/* direct SMI */
	else
		rpc.mgtclass = IB_SMI_CLASS;		/* Lid routed SMI */

	portid->sl = 0;
	portid->qp = 0;

	if (srcport) {
		return mad_rpc(srcport, &rpc, portid, 0, rcvbuf);
	} else {
		return madrpc(&rpc, portid, 0, rcvbuf);
	}
}

uint8_t *
smp_query(void *rcvbuf, ib_portid_t *portid, unsigned attrid, unsigned mod,
	  unsigned timeout)
{
	return smp_query_via(rcvbuf, portid, attrid, mod, timeout, NULL);
}
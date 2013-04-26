
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

static inline int
response_expected(int method)
{
	return method == IB_MAD_METHOD_GET ||
		method == IB_MAD_METHOD_SET ||
		method == IB_MAD_METHOD_TRAP;
}

uint8_t *
ib_vendor_call(void *data, ib_portid_t *portid, ib_vendor_call_t *call)
{
	ib_rpc_t rpc = {0};
	int range1 = 0, resp_expected;

	DEBUG("route %s data %p", portid2str(portid), data);
	if (portid->lid <= 0)
		return 0;	/* no direct SMI */

	if (!(range1 = mad_is_vendor_range1(call->mgmt_class)) &&
	    !(mad_is_vendor_range2(call->mgmt_class)))
		return 0;

	resp_expected = response_expected(call->method);

	rpc.mgtclass = call->mgmt_class;

	rpc.method = call->method;
	rpc.attr.id = call->attrid;
	rpc.attr.mod = call->mod;
	rpc.timeout = resp_expected ? call->timeout : 0;
	rpc.datasz = range1 ? IB_VENDOR_RANGE1_DATA_SIZE : IB_VENDOR_RANGE2_DATA_SIZE;
	rpc.dataoffs = range1 ? IB_VENDOR_RANGE1_DATA_OFFS : IB_VENDOR_RANGE2_DATA_OFFS;

	if (!range1)
		rpc.oui = call->oui;

	DEBUG("class 0x%x method 0x%x attr 0x%x mod 0x%x datasz %d off %d res_ex %d",
		rpc.mgtclass, rpc.method, rpc.attr.id, rpc.attr.mod,
		rpc.datasz, rpc.dataoffs, resp_expected);

	portid->qp = 1;
	if (!portid->qkey)
		portid->qkey = IB_DEFAULT_QP1_QKEY;

	if (resp_expected)
		return madrpc_rmpp(&rpc, portid, 0, data);		/* FIXME: no RMPP for now */

	return mad_send(&rpc, portid, 0, data) < 0 ? 0 : data;		/* FIXME: no RMPP for now */
}
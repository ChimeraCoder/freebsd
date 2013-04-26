
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

#include <infiniband/umad.h>
#include "mad.h"

#undef DEBUG
#define DEBUG 	if (ibdebug)	IBWARN

static uint8_t *
pma_query_via(void *rcvbuf, ib_portid_t *dest, int port,
	      unsigned timeout, unsigned id, const void *srcport)
{
	ib_rpc_t rpc = {0};
	int lid = dest->lid;

	DEBUG("lid %d port %d", lid, port);

	if (lid == -1) {
		IBWARN("only lid routed is supported");
		return 0;
	}

	rpc.mgtclass = IB_PERFORMANCE_CLASS;
	rpc.method = IB_MAD_METHOD_GET;
	rpc.attr.id = id;

	/* Same for attribute IDs */
	mad_set_field(rcvbuf, 0, IB_PC_PORT_SELECT_F, port);
	rpc.attr.mod = 0;
	rpc.timeout = timeout;
	rpc.datasz = IB_PC_DATA_SZ;
	rpc.dataoffs = IB_PC_DATA_OFFS;

	dest->qp = 1;
	if (!dest->qkey)
		dest->qkey = IB_DEFAULT_QP1_QKEY;

	if (srcport) {
		return mad_rpc(srcport, &rpc, dest, rcvbuf, rcvbuf);
	} else {
		return madrpc(&rpc, dest, rcvbuf, rcvbuf);
	}
}

uint8_t *
pma_query(void *rcvbuf, ib_portid_t *dest, int port, unsigned timeout, unsigned id)
{
	return pma_query_via(rcvbuf, dest, port, timeout, id, NULL);
}

uint8_t *
perf_classportinfo_query_via(void *rcvbuf, ib_portid_t *dest, int port,
			     unsigned timeout, const void *srcport)
{
	return pma_query_via(rcvbuf, dest, port, timeout, CLASS_PORT_INFO,
			     srcport);
}

uint8_t *
perf_classportinfo_query(void *rcvbuf, ib_portid_t *dest, int port, unsigned timeout)
{
	return pma_query(rcvbuf, dest, port, timeout, CLASS_PORT_INFO);
}

uint8_t *
port_performance_query_via(void *rcvbuf, ib_portid_t *dest, int port,
			   unsigned timeout, const void *srcport)
{
	return pma_query_via(rcvbuf, dest, port, timeout,
			     IB_GSI_PORT_COUNTERS, srcport);
}

uint8_t *
port_performance_query(void *rcvbuf, ib_portid_t *dest, int port, unsigned timeout)
{
	return pma_query(rcvbuf, dest, port, timeout, IB_GSI_PORT_COUNTERS);
}

static uint8_t *
performance_reset_via(void *rcvbuf, ib_portid_t *dest, int port, unsigned mask,
		      unsigned timeout, unsigned id, const void *srcport)
{
	ib_rpc_t rpc = {0};
	int lid = dest->lid;

	DEBUG("lid %d port %d mask 0x%x", lid, port, mask);

	if (lid == -1) {
		IBWARN("only lid routed is supported");
		return 0;
	}

	if (!mask)
		mask = ~0;

	rpc.mgtclass = IB_PERFORMANCE_CLASS;
	rpc.method = IB_MAD_METHOD_SET;
	rpc.attr.id = id;

	memset(rcvbuf, 0, IB_MAD_SIZE);

	/* Same for attribute IDs */
	mad_set_field(rcvbuf, 0, IB_PC_PORT_SELECT_F, port);
	mad_set_field(rcvbuf, 0, IB_PC_COUNTER_SELECT_F, mask);
	rpc.attr.mod = 0;
	rpc.timeout = timeout;
	rpc.datasz = IB_PC_DATA_SZ;
	rpc.dataoffs = IB_PC_DATA_OFFS;
	dest->qp = 1;
	if (!dest->qkey)
		dest->qkey = IB_DEFAULT_QP1_QKEY;

	if (srcport) {
		return mad_rpc(srcport, &rpc, dest, rcvbuf, rcvbuf);
	} else {
		return madrpc(&rpc, dest, rcvbuf, rcvbuf);
	}
}

static uint8_t *
performance_reset(void *rcvbuf, ib_portid_t *dest, int port, unsigned mask,
		  unsigned timeout, unsigned id)
{
	return performance_reset_via(rcvbuf, dest, port, mask, timeout,
				     id, NULL);
}

uint8_t *
port_performance_reset_via(void *rcvbuf, ib_portid_t *dest, int port,
			   unsigned mask, unsigned timeout, const void *srcport)
{
	return performance_reset_via(rcvbuf, dest, port, mask, timeout,
				     IB_GSI_PORT_COUNTERS, srcport);
}

uint8_t *
port_performance_reset(void *rcvbuf, ib_portid_t *dest, int port, unsigned mask,
		       unsigned timeout)
{
	return performance_reset(rcvbuf, dest, port, mask, timeout, IB_GSI_PORT_COUNTERS);
}

uint8_t *
port_performance_ext_query_via(void *rcvbuf, ib_portid_t *dest, int port,
			       unsigned timeout, const void *srcport)
{
	return pma_query_via(rcvbuf, dest, port, timeout,
			     IB_GSI_PORT_COUNTERS_EXT, srcport);
}

uint8_t *
port_performance_ext_query(void *rcvbuf, ib_portid_t *dest, int port, unsigned timeout)
{
	return pma_query(rcvbuf, dest, port, timeout, IB_GSI_PORT_COUNTERS_EXT);
}

uint8_t *
port_performance_ext_reset_via(void *rcvbuf, ib_portid_t *dest, int port,
			       unsigned mask, unsigned timeout,
			       const void *srcport)
{
	return performance_reset_via(rcvbuf, dest, port, mask, timeout,
				     IB_GSI_PORT_COUNTERS_EXT, srcport);
}

uint8_t *
port_performance_ext_reset(void *rcvbuf, ib_portid_t *dest, int port, unsigned mask,
			   unsigned timeout)
{
	return performance_reset(rcvbuf, dest, port, mask, timeout, IB_GSI_PORT_COUNTERS_EXT);
}

uint8_t *
port_samples_control_query_via(void *rcvbuf, ib_portid_t *dest, int port,
			       unsigned timeout, const void *srcport)
{
	return pma_query_via(rcvbuf, dest, port, timeout,
			     IB_GSI_PORT_SAMPLES_CONTROL, srcport);
}

uint8_t *
port_samples_control_query(void *rcvbuf, ib_portid_t *dest, int port, unsigned timeout)
{
	return pma_query(rcvbuf, dest, port, timeout, IB_GSI_PORT_SAMPLES_CONTROL);
}

uint8_t *
port_samples_result_query_via(void *rcvbuf, ib_portid_t *dest, int port,
			      unsigned timeout, const void *srcport)
{
	return pma_query_via(rcvbuf, dest, port, timeout,
			     IB_GSI_PORT_SAMPLES_RESULT, srcport);
}

uint8_t *
port_samples_result_query(void *rcvbuf, ib_portid_t *dest, int port,  unsigned timeout)
{
	return pma_query(rcvbuf, dest, port, timeout, IB_GSI_PORT_SAMPLES_RESULT);
}
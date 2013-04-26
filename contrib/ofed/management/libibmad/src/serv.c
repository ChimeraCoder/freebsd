
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
#include <pthread.h>
#include <sys/time.h>
#include <string.h>
#include <netinet/in.h>

#include <infiniband/common.h>
#include <infiniband/umad.h>
#include <mad.h>

#undef DEBUG
#define DEBUG	if (ibdebug)	IBWARN

int
mad_send(ib_rpc_t *rpc, ib_portid_t *dport, ib_rmpp_hdr_t *rmpp, void *data)
{
	uint8_t pktbuf[1024];
	void *umad = pktbuf;

	memset(pktbuf, 0, umad_size());

	DEBUG("rmpp %p data %p", rmpp, data);

	if (mad_build_pkt(umad, rpc, dport, rmpp, data) < 0)
		return 0;

	if (ibdebug) {
		IBWARN("data offs %d sz %d", rpc->dataoffs, rpc->datasz);
		xdump(stderr, "mad send data\n",
			(char *)umad_get_mad(umad) + rpc->dataoffs, rpc->datasz);
	}

	if (umad_send(madrpc_portid(), mad_class_agent(rpc->mgtclass),
		      umad, IB_MAD_SIZE, rpc->timeout, 0) < 0) {
		IBWARN("send failed; %m");
		return -1;
	}

	return 0;
}

int
mad_respond(void *umad, ib_portid_t *portid, uint32_t rstatus)
{
	uint8_t *mad = umad_get_mad(umad);
	ib_mad_addr_t *mad_addr;
	ib_rpc_t rpc = {0};
	ib_portid_t rport;
	int is_smi;

	if (!portid) {
		if (!(mad_addr = umad_get_mad_addr(umad)))
			return -1;

		memset(&rport, 0, sizeof(rport));

		rport.lid = ntohs(mad_addr->lid);
		rport.qp = ntohl(mad_addr->qpn);
		rport.qkey = ntohl(mad_addr->qkey);
		rport.sl = mad_addr->sl;

		portid = &rport;
	}

	DEBUG("dest %s", portid2str(portid));

	rpc.mgtclass = mad_get_field(mad, 0, IB_MAD_MGMTCLASS_F);

	rpc.method = mad_get_field(mad, 0, IB_MAD_METHOD_F);
	if (rpc.method == IB_MAD_METHOD_SET)
		rpc.method = IB_MAD_METHOD_GET;
	if (rpc.method != IB_MAD_METHOD_SEND)
		rpc.method |= IB_MAD_RESPONSE;

	rpc.attr.id = mad_get_field(mad, 0, IB_MAD_ATTRID_F);
	rpc.attr.mod = mad_get_field(mad, 0, IB_MAD_ATTRMOD_F);
	if (rpc.mgtclass == IB_SA_CLASS)
		rpc.recsz = mad_get_field(mad, 0, IB_SA_ATTROFFS_F);
	if (mad_is_vendor_range2(rpc.mgtclass))
		rpc.oui = mad_get_field(mad, 0, IB_VEND2_OUI_F);

	rpc.trid = mad_get_field64(mad, 0, IB_MAD_TRID_F);

	/* cleared by default: timeout, datasz, dataoffs, mkey, mask */

	is_smi = rpc.mgtclass == IB_SMI_CLASS ||
		 rpc.mgtclass == IB_SMI_DIRECT_CLASS;

	if (is_smi)
		portid->qp = 0;
	else if (!portid->qp)
		 portid->qp = 1;

	if (!portid->qkey && portid->qp == 1)
		portid->qkey = IB_DEFAULT_QP1_QKEY;

	DEBUG("qp 0x%x class 0x%x method %d attr 0x%x mod 0x%x datasz %d off %d qkey %x",
		portid->qp, rpc.mgtclass, rpc.method, rpc.attr.id, rpc.attr.mod,
		rpc.datasz, rpc.dataoffs, portid->qkey);

	if (mad_build_pkt(umad, &rpc, portid, 0, 0) < 0)
		return -1;

	if (ibdebug > 1)
		xdump(stderr, "mad respond pkt\n", mad, IB_MAD_SIZE);

	if (umad_send(madrpc_portid(), mad_class_agent(rpc.mgtclass), umad,
		      IB_MAD_SIZE, rpc.timeout, 0) < 0) {
		DEBUG("send failed; %m");
		return -1;
	}

	return 0;
}

void *
mad_receive(void *umad, int timeout)
{
	void *mad = umad ? umad : umad_alloc(1, umad_size() + IB_MAD_SIZE);
	int agent;
	int length = IB_MAD_SIZE;

	if ((agent = umad_recv(madrpc_portid(), mad,
			       &length, timeout)) < 0) {
		if (!umad)
			umad_free(mad);
		DEBUG("recv failed: %m");
		return 0;
	}

	return mad;
}

void *
mad_alloc(void)
{
	return umad_alloc(1, umad_size() + IB_MAD_SIZE);
}

void
mad_free(void *umad)
{
	umad_free(umad);
}
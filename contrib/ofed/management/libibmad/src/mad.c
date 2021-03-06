
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

#include <infiniband/common.h>
#include <infiniband/umad.h>
#include <mad.h>

#undef DEBUG
#define DEBUG	if (ibdebug)	IBWARN

void
mad_decode_field(uint8_t *buf, int field, void *val)
{
	ib_field_t *f = ib_mad_f + field;

	if (!field) {
		*(int *)val = *(int *)buf;
		return;
	}
	if (f->bitlen <= 32) {
		*(uint32_t *)val = _get_field(buf, 0, f);
		return;
	}
	if (f->bitlen == 64) {
		*(uint64_t *)val = _get_field64(buf, 0, f);
		return;
	}
	_get_array(buf, 0, f, val);
}

void
mad_encode_field(uint8_t *buf, int field, void *val)
{
	ib_field_t *f = ib_mad_f + field;

	if (!field) {
		*(int *)buf = *(int *)val;
		return;
	}
	if (f->bitlen <= 32) {
		_set_field(buf, 0, f, *(uint32_t *)val);
		return;
	}
	if (f->bitlen == 64) {
		_set_field64(buf, 0, f, *(uint64_t *)val);
		return;
	}
	_set_array(buf, 0, f, val);
}

uint64_t
mad_trid(void)
{
	static uint64_t base;
	static uint64_t trid;
	uint64_t next;

	if (!base) {
		srandom(time(0)*getpid());
		base = random();
		trid = random();
	}
	next = ++trid | (base << 32);
	return next;
}

void *
mad_encode(void *buf, ib_rpc_t *rpc, ib_dr_path_t *drpath, void *data)
{
	int is_resp = rpc->method & IB_MAD_RESPONSE;

	/* first word */
	mad_set_field(buf, 0, IB_MAD_METHOD_F, rpc->method);
	mad_set_field(buf, 0, IB_MAD_RESPONSE_F, is_resp ? 1 : 0);
	mad_set_field(buf, 0, IB_MAD_CLASSVER_F, rpc->mgtclass == IB_SA_CLASS ? 2 : 1);
	mad_set_field(buf, 0, IB_MAD_MGMTCLASS_F, rpc->mgtclass);
	mad_set_field(buf, 0, IB_MAD_BASEVER_F, 1);

	/* second word */
	if (rpc->mgtclass == IB_SMI_DIRECT_CLASS) {
		if (!drpath) {
			IBWARN("encoding dr mad without drpath (null)");
			return 0;
		}
		mad_set_field(buf, 0, IB_DRSMP_HOPCNT_F, drpath->cnt);
		mad_set_field(buf, 0, IB_DRSMP_HOPPTR_F, is_resp ? drpath->cnt + 1 : 0x0);
		mad_set_field(buf, 0, IB_DRSMP_STATUS_F, rpc->rstatus);
		mad_set_field(buf, 0, IB_DRSMP_DIRECTION_F, is_resp ? 1 : 0);	/* out */
	} else
		mad_set_field(buf, 0, IB_MAD_STATUS_F, rpc->rstatus);

	/* words 3,4,5,6 */
	if (!rpc->trid)
		rpc->trid = mad_trid();

	mad_set_field64(buf, 0, IB_MAD_TRID_F, rpc->trid);
	mad_set_field(buf, 0, IB_MAD_ATTRID_F, rpc->attr.id);
	mad_set_field(buf, 0, IB_MAD_ATTRMOD_F, rpc->attr.mod);

	/* words 7,8 */
	mad_set_field(buf, 0, IB_MAD_MKEY_F, rpc->mkey >> 32);
	mad_set_field(buf, 4, IB_MAD_MKEY_F, rpc->mkey & 0xffffffff);

	if (rpc->mgtclass == IB_SMI_DIRECT_CLASS) {
		/* word 9 */
		mad_set_field(buf, 0, IB_DRSMP_DRDLID_F, drpath->drdlid ? drpath->drdlid : 0xffff);
		mad_set_field(buf, 0, IB_DRSMP_DRSLID_F, drpath->drslid ? drpath->drslid : 0xffff);

		/* bytes 128 - 256 - by default should be zero due to memset*/
		if (is_resp)
			mad_set_array(buf, 0, IB_DRSMP_RPATH_F, drpath->p);
		else
			mad_set_array(buf, 0, IB_DRSMP_PATH_F, drpath->p);
	}

	if (rpc->mgtclass == IB_SA_CLASS)
		mad_set_field64(buf, 0, IB_SA_COMPMASK_F, rpc->mask);

	if (data)
		memcpy((char *)buf + rpc->dataoffs, data, rpc->datasz);

	/* vendor mads range 2 */
	if (mad_is_vendor_range2(rpc->mgtclass))
		mad_set_field(buf, 0, IB_VEND2_OUI_F, rpc->oui);

	return (uint8_t *)buf + IB_MAD_SIZE;
}

int
mad_build_pkt(void *umad, ib_rpc_t *rpc, ib_portid_t *dport,
	      ib_rmpp_hdr_t *rmpp, void *data)
{
	uint8_t *p, *mad;
	int lid_routed = rpc->mgtclass != IB_SMI_DIRECT_CLASS;
	int is_smi = (rpc->mgtclass == IB_SMI_CLASS ||
		      rpc->mgtclass == IB_SMI_DIRECT_CLASS);
	struct ib_mad_addr addr;

	if (!is_smi)
		umad_set_addr(umad, dport->lid, dport->qp, dport->sl, dport->qkey);
	else if (lid_routed)
		umad_set_addr(umad, dport->lid, dport->qp, 0, 0);
	else if ((dport->drpath.drslid != 0xffff) && (dport->lid > 0))
		umad_set_addr(umad, dport->lid, 0, 0, 0);
	else
		umad_set_addr(umad, 0xffff, 0, 0, 0);

	if (dport->grh_present && !is_smi) {
		addr.grh_present = 1;
		memcpy(addr.gid, dport->gid, 16);
		addr.hop_limit = 0xff;
		addr.traffic_class = 0;
		addr.flow_label = 0;
		umad_set_grh(umad, &addr);
	} else
		umad_set_grh(umad, 0);
	umad_set_pkey(umad, is_smi ? 0 : dport->pkey_idx);

	mad = umad_get_mad(umad);
	p = mad_encode(mad, rpc, lid_routed ? 0 : &dport->drpath, data);

	if (!is_smi && rmpp) {
		mad_set_field(mad, 0, IB_SA_RMPP_VERS_F, 1);
		mad_set_field(mad, 0, IB_SA_RMPP_TYPE_F, rmpp->type);
		mad_set_field(mad, 0, IB_SA_RMPP_RESP_F, 0x3f);
		mad_set_field(mad, 0, IB_SA_RMPP_FLAGS_F, rmpp->flags);
		mad_set_field(mad, 0, IB_SA_RMPP_STATUS_F, rmpp->status);
		mad_set_field(mad, 0, IB_SA_RMPP_D1_F, rmpp->d1.u);
		mad_set_field(mad, 0, IB_SA_RMPP_D2_F, rmpp->d2.u);
	}

	return p - mad;
}
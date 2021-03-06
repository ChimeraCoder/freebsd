
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

#include <string.h>

#include <infiniband/marshall.h>

void ibv_copy_ah_attr_from_kern(struct ibv_ah_attr *dst,
				struct ibv_kern_ah_attr *src)
{
	memcpy(dst->grh.dgid.raw, src->grh.dgid, sizeof dst->grh.dgid);
	dst->grh.flow_label = src->grh.flow_label;
	dst->grh.sgid_index = src->grh.sgid_index;
	dst->grh.hop_limit = src->grh.hop_limit;
	dst->grh.traffic_class = src->grh.traffic_class;

	dst->dlid = src->dlid;
	dst->sl = src->sl;
	dst->src_path_bits = src->src_path_bits;
	dst->static_rate = src->static_rate;
	dst->is_global = src->is_global;
	dst->port_num = src->port_num;
}

void ibv_copy_qp_attr_from_kern(struct ibv_qp_attr *dst,
				struct ibv_kern_qp_attr *src)
{
	dst->cur_qp_state = src->cur_qp_state;
	dst->path_mtu = src->path_mtu;
	dst->path_mig_state = src->path_mig_state;
	dst->qkey = src->qkey;
	dst->rq_psn = src->rq_psn;
	dst->sq_psn = src->sq_psn;
	dst->dest_qp_num = src->dest_qp_num;
	dst->qp_access_flags = src->qp_access_flags;

	dst->cap.max_send_wr = src->max_send_wr;
	dst->cap.max_recv_wr = src->max_recv_wr;
	dst->cap.max_send_sge = src->max_send_sge;
	dst->cap.max_recv_sge = src->max_recv_sge;
	dst->cap.max_inline_data = src->max_inline_data;

	ibv_copy_ah_attr_from_kern(&dst->ah_attr, &src->ah_attr);
	ibv_copy_ah_attr_from_kern(&dst->alt_ah_attr, &src->alt_ah_attr);

	dst->pkey_index = src->pkey_index;
	dst->alt_pkey_index = src->alt_pkey_index;
	dst->en_sqd_async_notify = src->en_sqd_async_notify;
	dst->sq_draining = src->sq_draining;
	dst->max_rd_atomic = src->max_rd_atomic;
	dst->max_dest_rd_atomic = src->max_dest_rd_atomic;
	dst->min_rnr_timer = src->min_rnr_timer;
	dst->port_num = src->port_num;
	dst->timeout = src->timeout;
	dst->retry_cnt = src->retry_cnt;
	dst->rnr_retry = src->rnr_retry;
	dst->alt_port_num = src->alt_port_num;
	dst->alt_timeout = src->alt_timeout;
}

void ibv_copy_path_rec_from_kern(struct ibv_sa_path_rec *dst,
				 struct ibv_kern_path_rec *src)
{
	memcpy(dst->dgid.raw, src->dgid, sizeof dst->dgid);
	memcpy(dst->sgid.raw, src->sgid, sizeof dst->sgid);

	dst->dlid		= src->dlid;
	dst->slid		= src->slid;
	dst->raw_traffic	= src->raw_traffic;
	dst->flow_label		= src->flow_label;
	dst->hop_limit		= src->hop_limit;
	dst->traffic_class	= src->traffic_class;
	dst->reversible		= src->reversible;
	dst->numb_path		= src->numb_path;
	dst->pkey		= src->pkey;
	dst->sl			= src->sl;
	dst->mtu_selector	= src->mtu_selector;
	dst->mtu		= src->mtu;
	dst->rate_selector	= src->rate_selector;
	dst->rate		= src->rate;
	dst->packet_life_time	= src->packet_life_time;
	dst->preference		= src->preference;
	dst->packet_life_time_selector = src->packet_life_time_selector;
}

void ibv_copy_path_rec_to_kern(struct ibv_kern_path_rec *dst,
			       struct ibv_sa_path_rec *src)
{
	memcpy(dst->dgid, src->dgid.raw, sizeof src->dgid);
	memcpy(dst->sgid, src->sgid.raw, sizeof src->sgid);

	dst->dlid		= src->dlid;
	dst->slid		= src->slid;
	dst->raw_traffic	= src->raw_traffic;
	dst->flow_label		= src->flow_label;
	dst->hop_limit		= src->hop_limit;
	dst->traffic_class	= src->traffic_class;
	dst->reversible		= src->reversible;
	dst->numb_path		= src->numb_path;
	dst->pkey		= src->pkey;
	dst->sl			= src->sl;
	dst->mtu_selector	= src->mtu_selector;
	dst->mtu		= src->mtu;
	dst->rate_selector	= src->rate_selector;
	dst->rate		= src->rate;
	dst->packet_life_time	= src->packet_life_time;
	dst->preference		= src->preference;
	dst->packet_life_time_selector = src->packet_life_time_selector;
}
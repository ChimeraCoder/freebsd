
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

/*
 * Abstract:
 *    Implementation of osm_mft_rcv_t.
 * This object represents the Multicast Forwarding Table Receiver object.
 * This object is part of the opensm family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <string.h>
#include <iba/ib_types.h>
#include <complib/cl_qmap.h>
#include <complib/cl_passivelock.h>
#include <complib/cl_debug.h>
#include <opensm/osm_madw.h>
#include <opensm/osm_log.h>
#include <opensm/osm_switch.h>
#include <opensm/osm_subnet.h>
#include <opensm/osm_sm.h>

/**********************************************************************
 **********************************************************************/
void osm_mft_rcv_process(IN void *context, IN void *data)
{
	osm_sm_t *sm = context;
	osm_madw_t *p_madw = data;
	ib_smp_t *p_smp;
	uint32_t block_num;
	uint8_t position;
	osm_switch_t *p_sw;
	osm_mft_context_t *p_mft_context;
	uint16_t *p_block;
	ib_net64_t node_guid;
	ib_api_status_t status;

	CL_ASSERT(sm);

	OSM_LOG_ENTER(sm->p_log);

	CL_ASSERT(p_madw);

	p_smp = osm_madw_get_smp_ptr(p_madw);
	p_block = (uint16_t *) ib_smp_get_payload_ptr(p_smp);
	block_num = cl_ntoh32(p_smp->attr_mod) & IB_MCAST_BLOCK_ID_MASK_HO;
	position = (uint8_t) ((cl_ntoh32(p_smp->attr_mod) &
			       IB_MCAST_POSITION_MASK_HO) >>
			      IB_MCAST_POSITION_SHIFT);

	/*
	   Acquire the switch object for this switch.
	 */
	p_mft_context = osm_madw_get_mft_context_ptr(p_madw);
	node_guid = p_mft_context->node_guid;

	OSM_LOG(sm->p_log, OSM_LOG_DEBUG,
		"Setting MFT block %u, position %u, "
		"Switch 0x%016" PRIx64 ", TID 0x%" PRIx64 "\n",
		block_num, position, cl_ntoh64(node_guid),
		cl_ntoh64(p_smp->trans_id));

	CL_PLOCK_EXCL_ACQUIRE(sm->p_lock);
	p_sw = osm_get_switch_by_guid(sm->p_subn, node_guid);

	if (!p_sw) {
		OSM_LOG(sm->p_log, OSM_LOG_ERROR, "ERR 0801: "
			"MFT received for nonexistent node "
			"0x%016" PRIx64 "\n", cl_ntoh64(node_guid));
	} else {
		status = osm_switch_set_mft_block(p_sw, p_block,
						  (uint16_t) block_num,
						  position);
		if (status != IB_SUCCESS) {
			OSM_LOG(sm->p_log, OSM_LOG_ERROR, "ERR 0802: "
				"Setting MFT block failed (%s)"
				"\n\t\t\t\tSwitch 0x%016" PRIx64
				", block %u, position %u\n",
				ib_get_err_str(status),
				cl_ntoh64(node_guid), block_num, position);
		}
	}

	CL_PLOCK_RELEASE(sm->p_lock);
	OSM_LOG_EXIT(sm->p_log);
}
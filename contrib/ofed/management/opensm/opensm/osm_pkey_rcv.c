
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
#endif				/* HAVE_CONFIG_H */

#include <string.h>
#include <iba/ib_types.h>
#include <complib/cl_passivelock.h>
#include <complib/cl_debug.h>
#include <opensm/osm_madw.h>
#include <opensm/osm_log.h>
#include <opensm/osm_node.h>
#include <opensm/osm_subnet.h>
#include <opensm/osm_helper.h>
#include <opensm/osm_sm.h>

/**********************************************************************
 **********************************************************************/
/*
 * WE MIGHT ONLY RECEIVE GET or SET responses
 */
void osm_pkey_rcv_process(IN void *context, IN void *data)
{
	osm_sm_t *sm = context;
	osm_madw_t *p_madw = data;
	ib_pkey_table_t *p_pkey_tbl;
	ib_smp_t *p_smp;
	osm_port_t *p_port;
	osm_physp_t *p_physp;
	osm_node_t *p_node;
	osm_pkey_context_t *p_context;
	ib_net64_t port_guid;
	ib_net64_t node_guid;
	uint8_t port_num;
	uint16_t block_num;

	CL_ASSERT(sm);

	OSM_LOG_ENTER(sm->p_log);

	CL_ASSERT(p_madw);

	p_smp = osm_madw_get_smp_ptr(p_madw);

	p_context = osm_madw_get_pkey_context_ptr(p_madw);
	p_pkey_tbl = (ib_pkey_table_t *) ib_smp_get_payload_ptr(p_smp);

	port_guid = p_context->port_guid;
	node_guid = p_context->node_guid;

	CL_ASSERT(p_smp->attr_id == IB_MAD_ATTR_P_KEY_TABLE);

	cl_plock_excl_acquire(sm->p_lock);
	p_port = osm_get_port_by_guid(sm->p_subn, port_guid);
	if (!p_port) {
		OSM_LOG(sm->p_log, OSM_LOG_ERROR, "ERR 4806: "
			"No port object for port with GUID 0x%" PRIx64
			"\n\t\t\t\tfor parent node GUID 0x%" PRIx64
			", TID 0x%" PRIx64 "\n",
			cl_ntoh64(port_guid),
			cl_ntoh64(node_guid), cl_ntoh64(p_smp->trans_id));
		goto Exit;
	}

	p_node = p_port->p_node;
	CL_ASSERT(p_node);

	block_num = (uint16_t) ((cl_ntoh32(p_smp->attr_mod)) & 0x0000FFFF);
	/* in case of a non switch node the attr modifier should be ignored */
	if (osm_node_get_type(p_node) == IB_NODE_TYPE_SWITCH) {
		port_num =
		    (uint8_t) (((cl_ntoh32(p_smp->attr_mod)) & 0x00FF0000) >>
			       16);
		p_physp = osm_node_get_physp_ptr(p_node, port_num);
	} else {
		p_physp = p_port->p_physp;
		port_num = p_physp->port_num;
	}

	/*
	   We do not mind if this is a result of a set or get - all we want is to
	   update the subnet.
	 */
	OSM_LOG(sm->p_log, OSM_LOG_VERBOSE,
		"Got GetResp(PKey) block:%u port_num %u with GUID 0x%"
		PRIx64 " for parent node GUID 0x%" PRIx64 ", TID 0x%"
		PRIx64 "\n", block_num, port_num, cl_ntoh64(port_guid),
		cl_ntoh64(node_guid), cl_ntoh64(p_smp->trans_id));

	/*
	   Determine if we encountered a new Physical Port.
	   If so, ignore it.
	 */
	if (!p_physp) {
		OSM_LOG(sm->p_log, OSM_LOG_ERROR, "ERR 4807: "
			"Got invalid port number %u\n", port_num);
		goto Exit;
	}

	osm_dump_pkey_block(sm->p_log,
			    port_guid, block_num,
			    port_num, p_pkey_tbl, OSM_LOG_DEBUG);

	osm_physp_set_pkey_tbl(sm->p_log, sm->p_subn,
			       p_physp, p_pkey_tbl, block_num);

Exit:
	cl_plock_release(sm->p_lock);

	OSM_LOG_EXIT(sm->p_log);
}
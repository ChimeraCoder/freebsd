
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
 *    Implementation of osm_nr_rcv_t.
 * This object represents the NodeInfo Receiver object.
 * This object is part of the opensm family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <string.h>
#include <iba/ib_types.h>
#include <complib/cl_debug.h>
#include <complib/cl_qlist.h>
#include <vendor/osm_vendor_api.h>
#include <opensm/osm_node.h>
#include <opensm/osm_helper.h>
#include <opensm/osm_pkey.h>
#include <opensm/osm_sa.h>

typedef struct osm_nr_item {
	cl_list_item_t list_item;
	ib_node_record_t rec;
} osm_nr_item_t;

typedef struct osm_nr_search_ctxt {
	const ib_node_record_t *p_rcvd_rec;
	ib_net64_t comp_mask;
	cl_qlist_t *p_list;
	osm_sa_t *sa;
	const osm_physp_t *p_req_physp;
} osm_nr_search_ctxt_t;

/**********************************************************************
 **********************************************************************/
static ib_api_status_t
__osm_nr_rcv_new_nr(IN osm_sa_t * sa,
		    IN const osm_node_t * const p_node,
		    IN cl_qlist_t * const p_list,
		    IN ib_net64_t const port_guid, IN ib_net16_t const lid)
{
	osm_nr_item_t *p_rec_item;
	ib_api_status_t status = IB_SUCCESS;

	OSM_LOG_ENTER(sa->p_log);

	p_rec_item = malloc(sizeof(*p_rec_item));
	if (p_rec_item == NULL) {
		OSM_LOG(sa->p_log, OSM_LOG_ERROR, "ERR 1D02: "
			"rec_item alloc failed\n");
		status = IB_INSUFFICIENT_RESOURCES;
		goto Exit;
	}

	OSM_LOG(sa->p_log, OSM_LOG_DEBUG,
		"New NodeRecord: node 0x%016" PRIx64
		"\n\t\t\t\tport 0x%016" PRIx64 ", lid %u\n",
		cl_ntoh64(osm_node_get_node_guid(p_node)),
		cl_ntoh64(port_guid), cl_ntoh16(lid));

	memset(p_rec_item, 0, sizeof(*p_rec_item));

	p_rec_item->rec.lid = lid;

	p_rec_item->rec.node_info = p_node->node_info;
	p_rec_item->rec.node_info.port_guid = port_guid;
	memcpy(&(p_rec_item->rec.node_desc), &(p_node->node_desc),
	       IB_NODE_DESCRIPTION_SIZE);
	cl_qlist_insert_tail(p_list, &p_rec_item->list_item);

Exit:
	OSM_LOG_EXIT(sa->p_log);
	return (status);
}

/**********************************************************************
 **********************************************************************/
static void
__osm_nr_rcv_create_nr(IN osm_sa_t * sa,
		       IN osm_node_t * const p_node,
		       IN cl_qlist_t * const p_list,
		       IN ib_net64_t const match_port_guid,
		       IN ib_net16_t const match_lid,
		       IN const osm_physp_t * const p_req_physp)
{
	const osm_physp_t *p_physp;
	uint8_t port_num;
	uint8_t num_ports;
	uint16_t match_lid_ho;
	ib_net16_t base_lid;
	ib_net16_t base_lid_ho;
	ib_net16_t max_lid_ho;
	uint8_t lmc;
	ib_net64_t port_guid;

	OSM_LOG_ENTER(sa->p_log);

	OSM_LOG(sa->p_log, OSM_LOG_DEBUG,
		"Looking for NodeRecord with LID: %u GUID:0x%016"
		PRIx64 "\n", cl_ntoh16(match_lid),
		cl_ntoh64(match_port_guid));

	/*
	   For switches, do not return the NodeInfo record
	   for each port on the switch, just for port 0.
	 */
	if (osm_node_get_type(p_node) == IB_NODE_TYPE_SWITCH)
		num_ports = 1;
	else
		num_ports = osm_node_get_num_physp(p_node);

	for (port_num = 0; port_num < num_ports; port_num++) {
		p_physp = osm_node_get_physp_ptr(p_node, port_num);
		if (!p_physp)
			continue;

		/* Check to see if the found p_physp and the requester physp
		   share a pkey. If not - continue */
		if (!osm_physp_share_pkey(sa->p_log, p_physp, p_req_physp))
			continue;

		port_guid = osm_physp_get_port_guid(p_physp);

		if (match_port_guid && (port_guid != match_port_guid))
			continue;

		base_lid = osm_physp_get_base_lid(p_physp);
		base_lid_ho = cl_ntoh16(base_lid);
		lmc = osm_physp_get_lmc(p_physp);
		max_lid_ho = (uint16_t) (base_lid_ho + (1 << lmc) - 1);
		match_lid_ho = cl_ntoh16(match_lid);

		if (match_lid_ho) {
			/*
			   We validate that the lid belongs to this node.
			 */
			OSM_LOG(sa->p_log, OSM_LOG_DEBUG,
				"Comparing LID: %u <= %u <= %u\n",
				base_lid_ho, match_lid_ho, max_lid_ho);

			if (match_lid_ho < base_lid_ho
			    || match_lid_ho > max_lid_ho)
				continue;
		}

		__osm_nr_rcv_new_nr(sa, p_node, p_list, port_guid, base_lid);

	}

	OSM_LOG_EXIT(sa->p_log);
}

/**********************************************************************
 **********************************************************************/
static void
__osm_nr_rcv_by_comp_mask(IN cl_map_item_t * const p_map_item, IN void *context)
{
	const osm_nr_search_ctxt_t *const p_ctxt =
	    (osm_nr_search_ctxt_t *) context;
	osm_node_t *const p_node = (osm_node_t *) p_map_item;
	const ib_node_record_t *const p_rcvd_rec = p_ctxt->p_rcvd_rec;
	const osm_physp_t *const p_req_physp = p_ctxt->p_req_physp;
	osm_sa_t *sa = p_ctxt->sa;
	ib_net64_t const comp_mask = p_ctxt->comp_mask;
	ib_net64_t match_port_guid = 0;
	ib_net16_t match_lid = 0;

	OSM_LOG_ENTER(p_ctxt->sa->p_log);

	osm_dump_node_info(p_ctxt->sa->p_log,
			   &p_node->node_info, OSM_LOG_VERBOSE);

	if (comp_mask & IB_NR_COMPMASK_LID)
		match_lid = p_rcvd_rec->lid;

	if (comp_mask & IB_NR_COMPMASK_NODEGUID) {
		OSM_LOG(sa->p_log, OSM_LOG_DEBUG,
			"Looking for node 0x%016" PRIx64
			", found 0x%016" PRIx64 "\n",
			cl_ntoh64(p_rcvd_rec->node_info.node_guid),
			cl_ntoh64(osm_node_get_node_guid(p_node)));

		if ((p_node->node_info.node_guid !=
		     p_rcvd_rec->node_info.node_guid))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_PORTGUID) {
		match_port_guid = p_rcvd_rec->node_info.port_guid;
	}
	if (comp_mask & IB_NR_COMPMASK_SYSIMAGEGUID) {
		if ((p_node->node_info.sys_guid !=
		     p_rcvd_rec->node_info.sys_guid))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_BASEVERSION) {
		if ((p_node->node_info.base_version !=
		     p_rcvd_rec->node_info.base_version))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_CLASSVERSION) {
		if ((p_node->node_info.class_version !=
		     p_rcvd_rec->node_info.class_version))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_NODETYPE) {
		if ((p_node->node_info.node_type !=
		     p_rcvd_rec->node_info.node_type))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_NUMPORTS) {
		if ((p_node->node_info.num_ports !=
		     p_rcvd_rec->node_info.num_ports))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_PARTCAP) {
		if ((p_node->node_info.partition_cap !=
		     p_rcvd_rec->node_info.partition_cap))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_DEVID) {
		if ((p_node->node_info.device_id !=
		     p_rcvd_rec->node_info.device_id))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_REV) {
		if ((p_node->node_info.revision !=
		     p_rcvd_rec->node_info.revision))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_PORTNUM) {
		if (ib_node_info_get_local_port_num(&p_node->node_info) !=
		    ib_node_info_get_local_port_num(&p_rcvd_rec->node_info))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_VENDID) {
		if (ib_node_info_get_vendor_id(&p_node->node_info) !=
		    ib_node_info_get_vendor_id(&p_rcvd_rec->node_info))
			goto Exit;
	}
	if (comp_mask & IB_NR_COMPMASK_NODEDESC) {
		if (strncmp((char *)&p_node->node_desc,
			    (char *)&p_rcvd_rec->node_desc,
			    sizeof(ib_node_desc_t)))
			goto Exit;
	}

	__osm_nr_rcv_create_nr(sa, p_node, p_ctxt->p_list,
			       match_port_guid, match_lid, p_req_physp);

Exit:
	OSM_LOG_EXIT(p_ctxt->sa->p_log);
}

/**********************************************************************
 **********************************************************************/
void osm_nr_rcv_process(IN void *ctx, IN void *data)
{
	osm_sa_t *sa = ctx;
	osm_madw_t *p_madw = data;
	const ib_sa_mad_t *p_rcvd_mad;
	const ib_node_record_t *p_rcvd_rec;
	cl_qlist_t rec_list;
	osm_nr_search_ctxt_t context;
	osm_physp_t *p_req_physp;

	CL_ASSERT(sa);

	OSM_LOG_ENTER(sa->p_log);

	CL_ASSERT(p_madw);

	p_rcvd_mad = osm_madw_get_sa_mad_ptr(p_madw);
	p_rcvd_rec = (ib_node_record_t *) ib_sa_mad_get_payload_ptr(p_rcvd_mad);

	CL_ASSERT(p_rcvd_mad->attr_id == IB_MAD_ATTR_NODE_RECORD);

	/* we only support SubnAdmGet and SubnAdmGetTable methods */
	if (p_rcvd_mad->method != IB_MAD_METHOD_GET &&
	    p_rcvd_mad->method != IB_MAD_METHOD_GETTABLE) {
		OSM_LOG(sa->p_log, OSM_LOG_ERROR, "ERR 1D05: "
			"Unsupported Method (%s)\n",
			ib_get_sa_method_str(p_rcvd_mad->method));
		osm_sa_send_error(sa, p_madw, IB_MAD_STATUS_UNSUP_METHOD_ATTR);
		goto Exit;
	}

	/* update the requester physical port. */
	p_req_physp = osm_get_physp_by_mad_addr(sa->p_log, sa->p_subn,
						osm_madw_get_mad_addr_ptr
						(p_madw));
	if (p_req_physp == NULL) {
		OSM_LOG(sa->p_log, OSM_LOG_ERROR, "ERR 1D04: "
			"Cannot find requester physical port\n");
		goto Exit;
	}

	if (osm_log_is_active(sa->p_log, OSM_LOG_DEBUG))
		osm_dump_node_record(sa->p_log, p_rcvd_rec, OSM_LOG_DEBUG);

	cl_qlist_init(&rec_list);

	context.p_rcvd_rec = p_rcvd_rec;
	context.p_list = &rec_list;
	context.comp_mask = p_rcvd_mad->comp_mask;
	context.sa = sa;
	context.p_req_physp = p_req_physp;

	cl_plock_acquire(sa->p_lock);

	cl_qmap_apply_func(&sa->p_subn->node_guid_tbl,
			   __osm_nr_rcv_by_comp_mask, &context);

	cl_plock_release(sa->p_lock);

	osm_sa_respond(sa, p_madw, sizeof(ib_node_record_t), &rec_list);

Exit:
	OSM_LOG_EXIT(sa->p_log);
}
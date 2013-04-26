
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
 *    Implementation of osm_nd_rcv_t.
 * This object represents the NodeDescription Receiver object.
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
#include <opensm/osm_node.h>
#include <opensm/osm_opensm.h>
#include <opensm/osm_subnet.h>

/**********************************************************************
 **********************************************************************/
static void
__osm_nd_rcv_process_nd(IN osm_sm_t * sm,
			IN osm_node_t * const p_node,
			IN const ib_node_desc_t * const p_nd)
{
	char *tmp_desc;
	char print_desc[IB_NODE_DESCRIPTION_SIZE + 1];

	OSM_LOG_ENTER(sm->p_log);

	memcpy(&p_node->node_desc.description, p_nd, sizeof(*p_nd));

	/* also set up a printable version */
	memcpy(print_desc, p_nd, sizeof(*p_nd));
	print_desc[IB_NODE_DESCRIPTION_SIZE] = '\0';
	tmp_desc = remap_node_name(sm->p_subn->p_osm->node_name_map,
			cl_ntoh64(osm_node_get_node_guid(p_node)),
			print_desc);

	/* make a copy for this node to "own" */
	if (p_node->print_desc)
		free(p_node->print_desc);
	p_node->print_desc = tmp_desc;

	OSM_LOG(sm->p_log, OSM_LOG_VERBOSE,
		"Node 0x%" PRIx64 "\n\t\t\t\tDescription = %s\n",
		cl_ntoh64(osm_node_get_node_guid(p_node)), p_node->print_desc);

	OSM_LOG_EXIT(sm->p_log);
}

/**********************************************************************
 **********************************************************************/
void osm_nd_rcv_process(IN void *context, IN void *data)
{
	osm_sm_t *sm = context;
	osm_madw_t *p_madw = data;
	ib_node_desc_t *p_nd;
	ib_smp_t *p_smp;
	osm_node_t *p_node;
	ib_net64_t node_guid;

	CL_ASSERT(sm);

	OSM_LOG_ENTER(sm->p_log);

	CL_ASSERT(p_madw);

	p_smp = osm_madw_get_smp_ptr(p_madw);
	p_nd = (ib_node_desc_t *) ib_smp_get_payload_ptr(p_smp);

	/*
	   Acquire the node object and add the node description.
	 */

	node_guid = osm_madw_get_nd_context_ptr(p_madw)->node_guid;
	CL_PLOCK_EXCL_ACQUIRE(sm->p_lock);
	p_node = osm_get_node_by_guid(sm->p_subn, node_guid);
	if (!p_node) {
		OSM_LOG(sm->p_log, OSM_LOG_ERROR, "ERR 0B01: "
			"NodeDescription received for nonexistent node "
			"0x%" PRIx64 "\n", cl_ntoh64(node_guid));
	} else {
		__osm_nd_rcv_process_nd(sm, p_node, p_nd);
	}

	CL_PLOCK_RELEASE(sm->p_lock);
	OSM_LOG_EXIT(sm->p_log);
}
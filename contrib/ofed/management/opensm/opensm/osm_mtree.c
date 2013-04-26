
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
 *    Implementation of osm_mtree_node_t.
 * This file implements the Multicast Tree object.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <complib/cl_debug.h>
#include <opensm/osm_mtree.h>

/**********************************************************************
 **********************************************************************/
static void
osm_mtree_node_init(IN osm_mtree_node_t * const p_mtn,
		    IN const osm_switch_t * const p_sw)
{
	uint32_t i;

	CL_ASSERT(p_mtn);
	CL_ASSERT(p_sw);

	memset(p_mtn, 0, sizeof(*p_mtn));

	p_mtn->p_sw = (osm_switch_t *) p_sw;
	p_mtn->max_children = p_sw->num_ports;

	for (i = 0; i < p_mtn->max_children; i++)
		p_mtn->child_array[i] = NULL;
}

/**********************************************************************
 **********************************************************************/
osm_mtree_node_t *osm_mtree_node_new(IN const osm_switch_t * const p_sw)
{
	osm_mtree_node_t *p_mtn;

	p_mtn = malloc(sizeof(osm_mtree_node_t) +
		       sizeof(void *) * (p_sw->num_ports - 1));

	if (p_mtn != NULL)
		osm_mtree_node_init(p_mtn, p_sw);

	return (p_mtn);
}

/**********************************************************************
 **********************************************************************/
void osm_mtree_destroy(IN osm_mtree_node_t * p_mtn)
{
	uint32_t i;

	if (p_mtn == NULL)
		return;

	if (p_mtn->child_array != NULL)
		for (i = 0; i < p_mtn->max_children; i++)
			if ((p_mtn->child_array[i] != NULL) &&
			    (p_mtn->child_array[i] != OSM_MTREE_LEAF))
				osm_mtree_destroy(p_mtn->child_array[i]);

	free(p_mtn);
}

/**********************************************************************
 **********************************************************************/
#if 0
static void __osm_mtree_dump(IN osm_mtree_node_t * p_mtn)
{
	uint32_t i;

	if (p_mtn == NULL)
		return;

	printf("GUID:0x%016" PRIx64 " max_children:%u\n",
	       cl_ntoh64(p_mtn->p_sw->p_node->node_info.node_guid),
	       p_mtn->max_children);
	if (p_mtn->child_array != NULL) {
		for (i = 0; i < p_mtn->max_children; i++) {
			printf("i=%d\n", i);
			if ((p_mtn->child_array[i] != NULL)
			    && (p_mtn->child_array[i] != OSM_MTREE_LEAF))
				__osm_mtree_dump(p_mtn->child_array[i]);
		}
	}
}
#endif

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
 *    Implementation of osm_router_t.
 * This object represents an Infiniband router.
 * This object is part of the opensm family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <iba/ib_types.h>
#include <opensm/osm_router.h>

/**********************************************************************
 **********************************************************************/
void osm_router_delete(IN OUT osm_router_t ** const pp_rtr)
{
	free(*pp_rtr);
	*pp_rtr = NULL;
}

/**********************************************************************
 **********************************************************************/
osm_router_t *osm_router_new(IN osm_port_t * const p_port)
{
	osm_router_t *p_rtr;

	CL_ASSERT(p_port);

	p_rtr = (osm_router_t *) malloc(sizeof(*p_rtr));
	if (p_rtr) {
		memset(p_rtr, 0, sizeof(*p_rtr));
		p_rtr->p_port = p_port;
	}

	return (p_rtr);
}
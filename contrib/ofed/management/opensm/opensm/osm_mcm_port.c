
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
 *    Implementation of osm_mcm_port_t.
 * This object represents the membership of a port in a multicast group.
 * This object is part of the OpenSM family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <string.h>
#include <opensm/osm_mcm_port.h>

/**********************************************************************
 **********************************************************************/
osm_mcm_port_t *osm_mcm_port_new(IN const ib_gid_t * const p_port_gid,
				 IN const uint8_t scope_state,
				 IN const boolean_t proxy_join)
{
	osm_mcm_port_t *p_mcm;

	p_mcm = malloc(sizeof(*p_mcm));
	if (p_mcm) {
		memset(p_mcm, 0, sizeof(*p_mcm));
		p_mcm->port_gid = *p_port_gid;
		p_mcm->scope_state = scope_state;
		p_mcm->proxy_join = proxy_join;
	}

	return (p_mcm);
}

/**********************************************************************
 **********************************************************************/
void osm_mcm_port_delete(IN osm_mcm_port_t * const p_mcm)
{
	CL_ASSERT(p_mcm);
	free(p_mcm);
}

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
 *    Declaration of osm_mcm_info_t.
 * This object represents a Multicast Forwarding Information object.
 * This object is part of the OpenSM family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <stdlib.h>
#include <opensm/osm_mcm_info.h>

/**********************************************************************
 **********************************************************************/
osm_mcm_info_t *osm_mcm_info_new(IN const ib_net16_t mlid)
{
	osm_mcm_info_t *p_mcm;

	p_mcm = (osm_mcm_info_t *) malloc(sizeof(*p_mcm));
	if (p_mcm) {
		memset(p_mcm, 0, sizeof(*p_mcm));
		p_mcm->mlid = mlid;
	}

	return (p_mcm);
}

/**********************************************************************
 **********************************************************************/
void osm_mcm_info_delete(IN osm_mcm_info_t * const p_mcm)
{
	free(p_mcm);
}
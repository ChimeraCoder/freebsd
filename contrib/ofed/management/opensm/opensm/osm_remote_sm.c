
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
 *    Implementation of osm_sm_t.
 * This object represents the remote SM object.
 * This object is part of the opensm family of objects.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <string.h>
#include <opensm/osm_remote_sm.h>

/**********************************************************************
 **********************************************************************/
void osm_remote_sm_construct(IN osm_remote_sm_t * const p_sm)
{
	memset(p_sm, 0, sizeof(*p_sm));
}

/**********************************************************************
 **********************************************************************/
void osm_remote_sm_destroy(IN osm_remote_sm_t * const p_sm)
{
	memset(p_sm, 0, sizeof(*p_sm));
}

/**********************************************************************
 **********************************************************************/
void
osm_remote_sm_init(IN osm_remote_sm_t * const p_sm,
		   IN const osm_port_t * const p_port,
		   IN const ib_sm_info_t * const p_smi)
{
	CL_ASSERT(p_sm);
	CL_ASSERT(p_port);

	osm_remote_sm_construct(p_sm);

	p_sm->p_port = p_port;
	p_sm->smi = *p_smi;
	return;
}
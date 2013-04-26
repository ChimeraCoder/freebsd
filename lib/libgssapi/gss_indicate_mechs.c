
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

#include <gssapi/gssapi.h>

#include "mech_switch.h"

OM_uint32
gss_indicate_mechs(OM_uint32 *minor_status,
    gss_OID_set *mech_set)
{
	struct _gss_mech_switch *m;
	OM_uint32 major_status;
	gss_OID_set set;
	size_t i;

	_gss_load_mech();

	major_status = gss_create_empty_oid_set(minor_status, mech_set);
	if (major_status)
		return (major_status);
	
	SLIST_FOREACH(m, &_gss_mechs, gm_link) {
		if (m->gm_indicate_mechs) {
			major_status = m->gm_indicate_mechs(minor_status, &set);
			if (major_status)
				continue;
			if (set == GSS_C_NO_OID_SET) {
				major_status = gss_add_oid_set_member(
					minor_status,
					&m->gm_mech_oid, mech_set);
				continue;
			}
			for (i = 0; i < set->count; i++)
				major_status = gss_add_oid_set_member(minor_status,
				    &set->elements[i], mech_set);
			gss_release_oid_set(minor_status, &set);
		} else {
			major_status = gss_add_oid_set_member(
			    minor_status, &m->gm_mech_oid, mech_set);
		}
	}

	*minor_status = 0;
	return (GSS_S_COMPLETE);
}
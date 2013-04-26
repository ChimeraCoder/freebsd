
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

#include "mech_locl.h"

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_inquire_mechs_for_name(OM_uint32 *minor_status,
    const gss_name_t input_name,
    gss_OID_set *mech_types)
{
	OM_uint32		major_status;
	struct _gss_name	*name = (struct _gss_name *) input_name;
	struct _gss_mech_switch	*m;
	gss_OID_set		name_types;
	int			present;

	*minor_status = 0;

	_gss_load_mech();

	major_status = gss_create_empty_oid_set(minor_status, mech_types);
	if (major_status)
		return (major_status);

	/*
	 * We go through all the loaded mechanisms and see if this
	 * name's type is supported by the mechanism. If it is, add
	 * the mechanism to the set.
	 */
	HEIM_SLIST_FOREACH(m, &_gss_mechs, gm_link) {
		major_status = gss_inquire_names_for_mech(minor_status,
		    &m->gm_mech_oid, &name_types);
		if (major_status) {
			gss_release_oid_set(minor_status, mech_types);
			return (major_status);
		}
		gss_test_oid_set_member(minor_status,
		    &name->gn_type, name_types, &present);
		gss_release_oid_set(minor_status, &name_types);
		if (present) {
			major_status = gss_add_oid_set_member(minor_status,
			    &m->gm_mech_oid, mech_types);
			if (major_status) {
				gss_release_oid_set(minor_status, mech_types);
				return (major_status);
			}
		}
	}

	return (GSS_S_COMPLETE);
}
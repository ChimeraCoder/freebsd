
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
gss_inquire_names_for_mech(OM_uint32 *minor_status,
    const gss_OID mechanism,
    gss_OID_set *name_types)
{
	OM_uint32 major_status;
	gssapi_mech_interface m = __gss_get_mechanism(mechanism);

	*minor_status = 0;
	*name_types = GSS_C_NO_OID_SET;
	if (!m)
		return (GSS_S_BAD_MECH);

	/*
	 * If the implementation can do it, ask it for a list of
	 * names, otherwise fake it.
	 */
	if (m->gm_inquire_names_for_mech) {
		return (m->gm_inquire_names_for_mech(minor_status,
			    mechanism, name_types));
	} else {
		major_status = gss_create_empty_oid_set(minor_status,
		    name_types);
		if (major_status)
			return (major_status);
		major_status = gss_add_oid_set_member(minor_status,
		    GSS_C_NT_HOSTBASED_SERVICE, name_types);
		if (major_status) {
			OM_uint32 junk;
			gss_release_oid_set(&junk, name_types);
			return (major_status);
		}
		major_status = gss_add_oid_set_member(minor_status,
		    GSS_C_NT_USER_NAME, name_types);
		if (major_status) {
			OM_uint32 junk;
			gss_release_oid_set(&junk, name_types);
			return (major_status);
		}
	}

	return (GSS_S_COMPLETE);
}
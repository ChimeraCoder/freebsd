
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

#include "gsskrb5_locl.h"

static gss_OID name_list[] = {
    GSS_C_NT_HOSTBASED_SERVICE,
    GSS_C_NT_USER_NAME,
    GSS_KRB5_NT_PRINCIPAL_NAME,
    GSS_C_NT_EXPORT_NAME,
    NULL
};

OM_uint32 GSSAPI_CALLCONV _gsskrb5_inquire_names_for_mech (
            OM_uint32 * minor_status,
            const gss_OID mechanism,
            gss_OID_set * name_types
           )
{
    OM_uint32 ret;
    int i;

    *minor_status = 0;

    if (gss_oid_equal(mechanism, GSS_KRB5_MECHANISM) == 0 &&
	gss_oid_equal(mechanism, GSS_C_NULL_OID) == 0) {
	*name_types = GSS_C_NO_OID_SET;
	return GSS_S_BAD_MECH;
    }

    ret = gss_create_empty_oid_set(minor_status, name_types);
    if (ret != GSS_S_COMPLETE)
	return ret;

    for (i = 0; name_list[i] != NULL; i++) {
	ret = gss_add_oid_set_member(minor_status,
				     name_list[i],
				     name_types);
	if (ret != GSS_S_COMPLETE)
	    break;
    }

    if (ret != GSS_S_COMPLETE)
	gss_release_oid_set(NULL, name_types);

    return GSS_S_COMPLETE;
}
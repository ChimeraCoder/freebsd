
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
gss_inquire_name(OM_uint32 *minor_status,
		 gss_name_t input_name,
		 int *name_is_MN,
		 gss_OID *MN_mech,
		 gss_buffer_set_t *attrs)
{
    OM_uint32 major_status = GSS_S_UNAVAILABLE;
    struct _gss_name *name = (struct _gss_name *) input_name;
    struct _gss_mechanism_name *mn;

    *minor_status = 0;
    if (name_is_MN != NULL)
        *name_is_MN = 0;
    if (MN_mech != NULL)
        *MN_mech = GSS_C_NO_OID;
    if (attrs != NULL)
        *attrs = GSS_C_NO_BUFFER_SET;

    if (input_name == GSS_C_NO_NAME)
        return GSS_S_BAD_NAME;

    HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
        gssapi_mech_interface m = mn->gmn_mech;

        if (!m->gm_inquire_name)
            continue;

        major_status = m->gm_inquire_name(minor_status,
                                          mn->gmn_name,
                                          NULL,
                                          MN_mech,
                                          attrs);
        if (major_status == GSS_S_COMPLETE) {
            if (name_is_MN != NULL)
                *name_is_MN = 1;
            if (MN_mech != NULL && *MN_mech == GSS_C_NO_OID)
                *MN_mech = &m->gm_mech_oid;
            break;
        }
        _gss_mg_error(m, major_status, *minor_status);
    }

    return major_status;
}

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
gss_set_name_attribute(OM_uint32 *minor_status,
		       gss_name_t input_name,
		       int complete,
		       gss_buffer_t attr,
		       gss_buffer_t value)
{
    OM_uint32 major_status = GSS_S_UNAVAILABLE;
    struct _gss_name *name = (struct _gss_name *) input_name;
    struct _gss_mechanism_name *mn;

    *minor_status = 0;

    if (input_name == GSS_C_NO_NAME)
        return GSS_S_BAD_NAME;

    HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
        gssapi_mech_interface m = mn->gmn_mech;

        if (!m->gm_set_name_attribute)
            continue;

        major_status = m->gm_set_name_attribute(minor_status,
                                                mn->gmn_name,
                                                complete,
                                                attr,
                                                value);
        if (GSS_ERROR(major_status))
            _gss_mg_error(m, major_status, *minor_status);
        else
            break;
    }

    return major_status;
}
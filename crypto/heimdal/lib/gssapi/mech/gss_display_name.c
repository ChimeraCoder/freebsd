
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
gss_display_name(OM_uint32 *minor_status,
    const gss_name_t input_name,
    gss_buffer_t output_name_buffer,
    gss_OID *output_name_type)
{
	OM_uint32 major_status;
	struct _gss_name *name = (struct _gss_name *) input_name;
	struct _gss_mechanism_name *mn;

	_mg_buffer_zero(output_name_buffer);
	if (output_name_type)
	    *output_name_type = GSS_C_NO_OID;

	if (name == NULL) {
		*minor_status = 0;
		return (GSS_S_BAD_NAME);
	}

	/*
	 * If we know it, copy the buffer used to import the name in
	 * the first place. Otherwise, ask all the MNs in turn if
	 * they can display the thing.
	 */
	if (name->gn_value.value) {
		output_name_buffer->value = malloc(name->gn_value.length);
		if (!output_name_buffer->value) {
			*minor_status = ENOMEM;
			return (GSS_S_FAILURE);
		}
		output_name_buffer->length = name->gn_value.length;
		memcpy(output_name_buffer->value, name->gn_value.value,
		    output_name_buffer->length);
		if (output_name_type)
			*output_name_type = &name->gn_type;

		*minor_status = 0;
		return (GSS_S_COMPLETE);
	} else {
		HEIM_SLIST_FOREACH(mn, &name->gn_mn, gmn_link) {
			major_status = mn->gmn_mech->gm_display_name(
				minor_status, mn->gmn_name,
				output_name_buffer,
				output_name_type);
			if (major_status == GSS_S_COMPLETE)
				return (GSS_S_COMPLETE);
		}
	}

	*minor_status = 0;
	return (GSS_S_FAILURE);
}
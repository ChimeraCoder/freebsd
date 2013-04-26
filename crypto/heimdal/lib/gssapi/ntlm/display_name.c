
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

#include "ntlm.h"

OM_uint32 GSSAPI_CALLCONV
_gss_ntlm_display_name
           (OM_uint32 * minor_status,
            const gss_name_t input_name,
            gss_buffer_t output_name_buffer,
            gss_OID * output_name_type
           )
{
    *minor_status = 0;

    if (output_name_type)
	*output_name_type = GSS_NTLM_MECHANISM;

    if (output_name_buffer) {
	ntlm_name n = (ntlm_name)input_name;
	char *str = NULL;
	int len;

	output_name_buffer->length = 0;
	output_name_buffer->value = NULL;

	if (n == NULL) {
	    *minor_status = 0;
	    return GSS_S_BAD_NAME;
	}

	len = asprintf(&str, "%s@%s", n->user, n->domain);
	if (len < 0 || str == NULL) {
	    *minor_status = ENOMEM;
	    return GSS_S_FAILURE;
	}
	output_name_buffer->length = len;
	output_name_buffer->value = str;
    }
    return GSS_S_COMPLETE;
}

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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_display_name
           (OM_uint32 * minor_status,
            const gss_name_t input_name,
            gss_buffer_t output_name_buffer,
            gss_OID * output_name_type
           )
{
    krb5_context context;
    krb5_const_principal name = (krb5_const_principal)input_name;
    krb5_error_code kret;
    char *buf;
    size_t len;

    GSSAPI_KRB5_INIT (&context);

    kret = krb5_unparse_name_flags (context, name,
				    KRB5_PRINCIPAL_UNPARSE_DISPLAY, &buf);
    if (kret) {
	*minor_status = kret;
	return GSS_S_FAILURE;
    }
    len = strlen (buf);
    output_name_buffer->length = len;
    output_name_buffer->value  = malloc(len + 1);
    if (output_name_buffer->value == NULL) {
	free (buf);
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }
    memcpy (output_name_buffer->value, buf, len);
    ((char *)output_name_buffer->value)[len] = '\0';
    free (buf);
    if (output_name_type)
	*output_name_type = GSS_KRB5_NT_PRINCIPAL_NAME;
    *minor_status = 0;
    return GSS_S_COMPLETE;
}
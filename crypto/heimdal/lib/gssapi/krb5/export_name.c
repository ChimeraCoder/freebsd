
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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_export_name
           (OM_uint32  * minor_status,
            const gss_name_t input_name,
            gss_buffer_t exported_name
           )
{
    krb5_context context;
    krb5_const_principal princ = (krb5_const_principal)input_name;
    krb5_error_code kret;
    char *buf, *name;
    size_t len;

    GSSAPI_KRB5_INIT (&context);

    kret = krb5_unparse_name (context, princ, &name);
    if (kret) {
	*minor_status = kret;
	return GSS_S_FAILURE;
    }
    len = strlen (name);

    exported_name->length = 10 + len + GSS_KRB5_MECHANISM->length;
    exported_name->value  = malloc(exported_name->length);
    if (exported_name->value == NULL) {
	free (name);
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    /* TOK, MECH_OID_LEN, DER(MECH_OID), NAME_LEN, NAME */

    buf = exported_name->value;
    memcpy(buf, "\x04\x01", 2);
    buf += 2;
    buf[0] = ((GSS_KRB5_MECHANISM->length + 2) >> 8) & 0xff;
    buf[1] = (GSS_KRB5_MECHANISM->length + 2) & 0xff;
    buf+= 2;
    buf[0] = 0x06;
    buf[1] = (GSS_KRB5_MECHANISM->length) & 0xFF;
    buf+= 2;

    memcpy(buf, GSS_KRB5_MECHANISM->elements, GSS_KRB5_MECHANISM->length);
    buf += GSS_KRB5_MECHANISM->length;

    buf[0] = (len >> 24) & 0xff;
    buf[1] = (len >> 16) & 0xff;
    buf[2] = (len >> 8) & 0xff;
    buf[3] = (len) & 0xff;
    buf += 4;

    memcpy (buf, name, len);

    free (name);

    *minor_status = 0;
    return GSS_S_COMPLETE;
}
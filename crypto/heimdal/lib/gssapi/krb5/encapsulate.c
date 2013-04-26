
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

void
_gssapi_encap_length (size_t data_len,
		      size_t *len,
		      size_t *total_len,
		      const gss_OID mech)
{
    size_t len_len;

    *len = 1 + 1 + mech->length + data_len;

    len_len = der_length_len(*len);

    *total_len = 1 + len_len + *len;
}

void
_gsskrb5_encap_length (size_t data_len,
			  size_t *len,
			  size_t *total_len,
			  const gss_OID mech)
{
    _gssapi_encap_length(data_len + 2, len, total_len, mech);
}

void *
_gsskrb5_make_header (void *ptr,
			 size_t len,
			 const void *type,
			 const gss_OID mech)
{
    u_char *p = ptr;
    p = _gssapi_make_mech_header(p, len, mech);
    memcpy (p, type, 2);
    p += 2;
    return p;
}

void *
_gssapi_make_mech_header(void *ptr,
			 size_t len,
			 const gss_OID mech)
{
    u_char *p = ptr;
    int e;
    size_t len_len, foo;

    *p++ = 0x60;
    len_len = der_length_len(len);
    e = der_put_length (p + len_len - 1, len_len, len, &foo);
    if(e || foo != len_len)
	abort ();
    p += len_len;
    *p++ = 0x06;
    *p++ = mech->length;
    memcpy (p, mech->elements, mech->length);
    p += mech->length;
    return p;
}

/*
 * Give it a krb5_data and it will encapsulate with extra GSS-API wrappings.
 */

OM_uint32
_gssapi_encapsulate(
    OM_uint32 *minor_status,
    const krb5_data *in_data,
    gss_buffer_t output_token,
    const gss_OID mech
)
{
    size_t len, outer_len;
    void *p;

    _gssapi_encap_length (in_data->length, &len, &outer_len, mech);

    output_token->length = outer_len;
    output_token->value  = malloc (outer_len);
    if (output_token->value == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    p = _gssapi_make_mech_header (output_token->value, len, mech);
    memcpy (p, in_data->data, in_data->length);
    return GSS_S_COMPLETE;
}

/*
 * Give it a krb5_data and it will encapsulate with extra GSS-API krb5
 * wrappings.
 */

OM_uint32
_gsskrb5_encapsulate(
			OM_uint32 *minor_status,
			const krb5_data *in_data,
			gss_buffer_t output_token,
			const void *type,
			const gss_OID mech
)
{
    size_t len, outer_len;
    u_char *p;

    _gsskrb5_encap_length (in_data->length, &len, &outer_len, mech);

    output_token->length = outer_len;
    output_token->value  = malloc (outer_len);
    if (output_token->value == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    p = _gsskrb5_make_header (output_token->value, len, type, mech);
    memcpy (p, in_data->data, in_data->length);
    return GSS_S_COMPLETE;
}
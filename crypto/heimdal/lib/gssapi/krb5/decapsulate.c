
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

/*
 * return the length of the mechanism in token or -1
 * (which implies that the token was bad - GSS_S_DEFECTIVE_TOKEN
 */

ssize_t
_gsskrb5_get_mech (const u_char *ptr,
		      size_t total_len,
		      const u_char **mech_ret)
{
    size_t len, len_len, mech_len, foo;
    const u_char *p = ptr;
    int e;

    if (total_len < 1)
	return -1;
    if (*p++ != 0x60)
	return -1;
    e = der_get_length (p, total_len - 1, &len, &len_len);
    if (e || 1 + len_len + len != total_len)
	return -1;
    p += len_len;
    if (*p++ != 0x06)
	return -1;
    e = der_get_length (p, total_len - 1 - len_len - 1,
			&mech_len, &foo);
    if (e)
	return -1;
    p += foo;
    *mech_ret = p;
    return mech_len;
}

OM_uint32
_gssapi_verify_mech_header(u_char **str,
			   size_t total_len,
			   gss_OID mech)
{
    const u_char *p;
    ssize_t mech_len;

    mech_len = _gsskrb5_get_mech (*str, total_len, &p);
    if (mech_len < 0)
	return GSS_S_DEFECTIVE_TOKEN;

    if (mech_len != mech->length)
	return GSS_S_BAD_MECH;
    if (ct_memcmp(p,
		  mech->elements,
		  mech->length) != 0)
	return GSS_S_BAD_MECH;
    p += mech_len;
    *str = rk_UNCONST(p);
    return GSS_S_COMPLETE;
}

OM_uint32
_gsskrb5_verify_header(u_char **str,
			  size_t total_len,
			  const void *type,
			  gss_OID oid)
{
    OM_uint32 ret;
    size_t len;
    u_char *p = *str;

    ret = _gssapi_verify_mech_header(str, total_len, oid);
    if (ret)
	return ret;

    len = total_len - (*str - p);

    if (len < 2)
	return GSS_S_DEFECTIVE_TOKEN;

    if (ct_memcmp (*str, type, 2) != 0)
	return GSS_S_DEFECTIVE_TOKEN;
    *str += 2;

    return 0;
}

/*
 * Remove the GSS-API wrapping from `in_token' giving `out_data.
 * Does not copy data, so just free `in_token'.
 */

OM_uint32
_gssapi_decapsulate(
    OM_uint32 *minor_status,
    gss_buffer_t input_token_buffer,
    krb5_data *out_data,
    const gss_OID mech
)
{
    u_char *p;
    OM_uint32 ret;

    p = input_token_buffer->value;
    ret = _gssapi_verify_mech_header(&p,
				    input_token_buffer->length,
				    mech);
    if (ret) {
	*minor_status = 0;
	return ret;
    }

    out_data->length = input_token_buffer->length -
	(p - (u_char *)input_token_buffer->value);
    out_data->data   = p;
    return GSS_S_COMPLETE;
}

/*
 * Remove the GSS-API wrapping from `in_token' giving `out_data.
 * Does not copy data, so just free `in_token'.
 */

OM_uint32
_gsskrb5_decapsulate(OM_uint32 *minor_status,
			gss_buffer_t input_token_buffer,
			krb5_data *out_data,
			const void *type,
			gss_OID oid)
{
    u_char *p;
    OM_uint32 ret;

    p = input_token_buffer->value;
    ret = _gsskrb5_verify_header(&p,
				    input_token_buffer->length,
				    type,
				    oid);
    if (ret) {
	*minor_status = 0;
	return ret;
    }

    out_data->length = input_token_buffer->length -
	(p - (u_char *)input_token_buffer->value);
    out_data->data   = p;
    return GSS_S_COMPLETE;
}

/*
 * Verify padding of a gss wrapped message and return its length.
 */

OM_uint32
_gssapi_verify_pad(gss_buffer_t wrapped_token,
		   size_t datalen,
		   size_t *padlen)
{
    u_char *pad;
    size_t padlength;
    int i;

    pad = (u_char *)wrapped_token->value + wrapped_token->length - 1;
    padlength = *pad;

    if (padlength > datalen)
	return GSS_S_BAD_MECH;

    for (i = padlength; i > 0 && *pad == padlength; i--, pad--)
	;
    if (i != 0)
	return GSS_S_BAD_MIC;

    *padlen = padlength;

    return 0;
}
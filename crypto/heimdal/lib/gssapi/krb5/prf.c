
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

OM_uint32 GSSAPI_CALLCONV
_gsskrb5_pseudo_random(OM_uint32 *minor_status,
		       gss_ctx_id_t context_handle,
		       int prf_key,
		       const gss_buffer_t prf_in,
		       ssize_t desired_output_len,
		       gss_buffer_t prf_out)
{
    gsskrb5_ctx ctx = (gsskrb5_ctx)context_handle;
    krb5_context context;
    krb5_error_code ret;
    krb5_crypto crypto;
    krb5_data input, output;
    uint32_t num;
    OM_uint32 junk;
    unsigned char *p;
    krb5_keyblock *key = NULL;
    size_t dol;

    if (ctx == NULL) {
	*minor_status = 0;
	return GSS_S_NO_CONTEXT;
    }

    if (desired_output_len <= 0 || prf_in->length + 4 < prf_in->length) {
	*minor_status = 0;
	return GSS_S_FAILURE;
    }
    dol = desired_output_len;

    GSSAPI_KRB5_INIT (&context);

    switch(prf_key) {
    case GSS_C_PRF_KEY_FULL:
	_gsskrb5i_get_acceptor_subkey(ctx, context, &key);
	break;
    case GSS_C_PRF_KEY_PARTIAL:
	_gsskrb5i_get_initiator_subkey(ctx, context, &key);
	break;
    default:
	_gsskrb5_set_status(EINVAL, "unknown kerberos prf_key");
	*minor_status = EINVAL;
	return GSS_S_FAILURE;
    }

    if (key == NULL) {
	_gsskrb5_set_status(EINVAL, "no prf_key found");
	*minor_status = EINVAL;
	return GSS_S_FAILURE;
    }

    ret = krb5_crypto_init(context, key, 0, &crypto);
    krb5_free_keyblock (context, key);
    if (ret) {
	*minor_status = ret;
	return GSS_S_FAILURE;
    }

    prf_out->value = malloc(dol);
    if (prf_out->value == NULL) {
	_gsskrb5_set_status(GSS_KRB5_S_KG_INPUT_TOO_LONG, "Out of memory");
	*minor_status = GSS_KRB5_S_KG_INPUT_TOO_LONG;
	krb5_crypto_destroy(context, crypto);
	return GSS_S_FAILURE;
    }
    prf_out->length = dol;

    HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);

    input.length = prf_in->length + 4;
    input.data = malloc(prf_in->length + 4);
    if (input.data == NULL) {
	_gsskrb5_set_status(GSS_KRB5_S_KG_INPUT_TOO_LONG, "Out of memory");
	*minor_status = GSS_KRB5_S_KG_INPUT_TOO_LONG;
	gss_release_buffer(&junk, prf_out);
	krb5_crypto_destroy(context, crypto);
	HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);
	return GSS_S_FAILURE;
    }
    memcpy(((uint8_t *)input.data) + 4, prf_in->value, prf_in->length);

    num = 0;
    p = prf_out->value;
    while(dol > 0) {
	size_t tsize;

	_gsskrb5_encode_om_uint32(num, input.data);

	ret = krb5_crypto_prf(context, crypto, &input, &output);
	if (ret) {
	    *minor_status = ret;
	    free(input.data);
	    gss_release_buffer(&junk, prf_out);
	    krb5_crypto_destroy(context, crypto);
	    HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);
	    return GSS_S_FAILURE;
	}

	tsize = min(dol, output.length);
	memcpy(p, output.data, tsize);
	p += output.length;
	dol -= tsize;
	krb5_data_free(&output);
	num++;
    }
    free(input.data);

    krb5_crypto_destroy(context, crypto);

    HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);

    return GSS_S_COMPLETE;
}
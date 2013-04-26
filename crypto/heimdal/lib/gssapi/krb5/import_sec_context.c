
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
_gsskrb5_import_sec_context (
    OM_uint32 * minor_status,
    const gss_buffer_t interprocess_token,
    gss_ctx_id_t * context_handle
    )
{
    OM_uint32 ret = GSS_S_FAILURE;
    krb5_context context;
    krb5_error_code kret;
    krb5_storage *sp;
    krb5_auth_context ac;
    krb5_address local, remote;
    krb5_address *localp, *remotep;
    krb5_data data;
    gss_buffer_desc buffer;
    krb5_keyblock keyblock;
    int32_t flags, tmp;
    gsskrb5_ctx ctx;
    gss_name_t name;

    GSSAPI_KRB5_INIT (&context);

    *context_handle = GSS_C_NO_CONTEXT;

    localp = remotep = NULL;

    sp = krb5_storage_from_mem (interprocess_token->value,
				interprocess_token->length);
    if (sp == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    ctx = calloc(1, sizeof(*ctx));
    if (ctx == NULL) {
	*minor_status = ENOMEM;
	krb5_storage_free (sp);
	return GSS_S_FAILURE;
    }
    HEIMDAL_MUTEX_init(&ctx->ctx_id_mutex);

    kret = krb5_auth_con_init (context,
			       &ctx->auth_context);
    if (kret) {
	*minor_status = kret;
	ret = GSS_S_FAILURE;
	goto failure;
    }

    /* flags */

    *minor_status = 0;

    if (krb5_ret_int32 (sp, &flags) != 0)
	goto failure;

    /* retrieve the auth context */

    ac = ctx->auth_context;
    if (krb5_ret_int32 (sp, &tmp) != 0)
	goto failure;
    ac->flags = tmp;
    if (flags & SC_LOCAL_ADDRESS) {
	if (krb5_ret_address (sp, localp = &local) != 0)
	    goto failure;
    }

    if (flags & SC_REMOTE_ADDRESS) {
	if (krb5_ret_address (sp, remotep = &remote) != 0)
	    goto failure;
    }

    krb5_auth_con_setaddrs (context, ac, localp, remotep);
    if (localp)
	krb5_free_address (context, localp);
    if (remotep)
	krb5_free_address (context, remotep);
    localp = remotep = NULL;

    if (krb5_ret_int16 (sp, &ac->local_port) != 0)
	goto failure;

    if (krb5_ret_int16 (sp, &ac->remote_port) != 0)
	goto failure;
    if (flags & SC_KEYBLOCK) {
	if (krb5_ret_keyblock (sp, &keyblock) != 0)
	    goto failure;
	krb5_auth_con_setkey (context, ac, &keyblock);
	krb5_free_keyblock_contents (context, &keyblock);
    }
    if (flags & SC_LOCAL_SUBKEY) {
	if (krb5_ret_keyblock (sp, &keyblock) != 0)
	    goto failure;
	krb5_auth_con_setlocalsubkey (context, ac, &keyblock);
	krb5_free_keyblock_contents (context, &keyblock);
    }
    if (flags & SC_REMOTE_SUBKEY) {
	if (krb5_ret_keyblock (sp, &keyblock) != 0)
	    goto failure;
	krb5_auth_con_setremotesubkey (context, ac, &keyblock);
	krb5_free_keyblock_contents (context, &keyblock);
    }
    if (krb5_ret_uint32 (sp, &ac->local_seqnumber))
	goto failure;
    if (krb5_ret_uint32 (sp, &ac->remote_seqnumber))
	goto failure;

    if (krb5_ret_int32 (sp, &tmp) != 0)
	goto failure;
    ac->keytype = tmp;
    if (krb5_ret_int32 (sp, &tmp) != 0)
	goto failure;
    ac->cksumtype = tmp;

    /* names */

    if (krb5_ret_data (sp, &data))
	goto failure;
    buffer.value  = data.data;
    buffer.length = data.length;

    ret = _gsskrb5_import_name (minor_status, &buffer, GSS_C_NT_EXPORT_NAME,
				&name);
    if (ret) {
	ret = _gsskrb5_import_name (minor_status, &buffer, GSS_C_NO_OID,
				    &name);
	if (ret) {
	    krb5_data_free (&data);
	    goto failure;
	}
    }
    ctx->source = (krb5_principal)name;
    krb5_data_free (&data);

    if (krb5_ret_data (sp, &data) != 0)
	goto failure;
    buffer.value  = data.data;
    buffer.length = data.length;

    ret = _gsskrb5_import_name (minor_status, &buffer, GSS_C_NT_EXPORT_NAME,
				&name);
    if (ret) {
	ret = _gsskrb5_import_name (minor_status, &buffer, GSS_C_NO_OID,
				    &name);
	if (ret) {
	    krb5_data_free (&data);
	    goto failure;
	}
    }
    ctx->target = (krb5_principal)name;
    krb5_data_free (&data);

    if (krb5_ret_int32 (sp, &tmp))
	goto failure;
    ctx->flags = tmp;
    if (krb5_ret_int32 (sp, &tmp))
	goto failure;
    ctx->more_flags = tmp;
    if (krb5_ret_int32 (sp, &tmp))
	goto failure;
    ctx->lifetime = tmp;

    ret = _gssapi_msg_order_import(minor_status, sp, &ctx->order);
    if (ret)
        goto failure;

    krb5_storage_free (sp);

    _gsskrb5i_is_cfx(context, ctx, (ctx->more_flags & LOCAL) == 0);

    *context_handle = (gss_ctx_id_t)ctx;

    return GSS_S_COMPLETE;

failure:
    krb5_auth_con_free (context,
			ctx->auth_context);
    if (ctx->source != NULL)
	krb5_free_principal(context, ctx->source);
    if (ctx->target != NULL)
	krb5_free_principal(context, ctx->target);
    if (localp)
	krb5_free_address (context, localp);
    if (remotep)
	krb5_free_address (context, remotep);
    if(ctx->order)
	_gssapi_msg_order_destroy(&ctx->order);
    HEIMDAL_MUTEX_destroy(&ctx->ctx_id_mutex);
    krb5_storage_free (sp);
    free (ctx);
    *context_handle = GSS_C_NO_CONTEXT;
    return ret;
}
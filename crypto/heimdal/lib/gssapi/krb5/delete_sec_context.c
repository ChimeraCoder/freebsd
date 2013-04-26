
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
_gsskrb5_delete_sec_context(OM_uint32 * minor_status,
			    gss_ctx_id_t * context_handle,
			    gss_buffer_t output_token)
{
    krb5_context context;
    gsskrb5_ctx ctx;

    GSSAPI_KRB5_INIT (&context);

    *minor_status = 0;

    if (output_token) {
	output_token->length = 0;
	output_token->value  = NULL;
    }

    if (*context_handle == GSS_C_NO_CONTEXT)
	return GSS_S_COMPLETE;

    ctx = (gsskrb5_ctx) *context_handle;
    *context_handle = GSS_C_NO_CONTEXT;

    HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);

    krb5_auth_con_free (context, ctx->auth_context);
    krb5_auth_con_free (context, ctx->deleg_auth_context);
    if (ctx->kcred)
	krb5_free_creds(context, ctx->kcred);
    if(ctx->source)
	krb5_free_principal (context, ctx->source);
    if(ctx->target)
	krb5_free_principal (context, ctx->target);
    if (ctx->ticket)
	krb5_free_ticket (context, ctx->ticket);
    if(ctx->order)
	_gssapi_msg_order_destroy(&ctx->order);
    if (ctx->service_keyblock)
	krb5_free_keyblock (context, ctx->service_keyblock);
    krb5_data_free(&ctx->fwd_data);
    if (ctx->crypto)
    	krb5_crypto_destroy(context, ctx->crypto);

    HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);
    HEIMDAL_MUTEX_destroy(&ctx->ctx_id_mutex);
    memset(ctx, 0, sizeof(*ctx));
    free (ctx);
    return GSS_S_COMPLETE;
}
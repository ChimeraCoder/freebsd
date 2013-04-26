
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

OM_uint32 GSSAPI_CALLCONV _gss_ntlm_delete_sec_context
           (OM_uint32 * minor_status,
            gss_ctx_id_t * context_handle,
            gss_buffer_t output_token
           )
{
    if (context_handle) {
	ntlm_ctx ctx = (ntlm_ctx)*context_handle;
	gss_cred_id_t cred = (gss_cred_id_t)ctx->client;

	*context_handle = GSS_C_NO_CONTEXT;

	if (ctx->server)
	    (*ctx->server->nsi_destroy)(minor_status, ctx->ictx);

	_gss_ntlm_release_cred(NULL, &cred);

	memset(ctx, 0, sizeof(*ctx));
	free(ctx);
    }
    if (output_token) {
	output_token->length = 0;
	output_token->value  = NULL;
    }

    *minor_status = 0;
    return GSS_S_COMPLETE;
}
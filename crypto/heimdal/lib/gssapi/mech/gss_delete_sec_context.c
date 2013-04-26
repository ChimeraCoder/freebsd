
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
gss_delete_sec_context(OM_uint32 *minor_status,
    gss_ctx_id_t *context_handle,
    gss_buffer_t output_token)
{
	OM_uint32 major_status;
	struct _gss_context *ctx = (struct _gss_context *) *context_handle;

	if (output_token)
	    _mg_buffer_zero(output_token);

	*minor_status = 0;
	if (ctx) {
		/*
		 * If we have an implementation ctx, delete it,
		 * otherwise fake an empty token.
		 */
		if (ctx->gc_ctx) {
			major_status = ctx->gc_mech->gm_delete_sec_context(
				minor_status, &ctx->gc_ctx, output_token);
		}
		free(ctx);
		*context_handle = GSS_C_NO_CONTEXT;
	}

	return (GSS_S_COMPLETE);
}
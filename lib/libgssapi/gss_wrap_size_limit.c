
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

#include <gssapi/gssapi.h>

#include "mech_switch.h"
#include "context.h"

OM_uint32
gss_wrap_size_limit(OM_uint32 *minor_status,
    const gss_ctx_id_t context_handle,
    int conf_req_flag,
    gss_qop_t qop_req,
    OM_uint32 req_output_size,
    OM_uint32 *max_input_size)
{
	struct _gss_context *ctx = (struct _gss_context *) context_handle;
	struct _gss_mech_switch *m = ctx->gc_mech;

	*max_input_size = 0;
	if (ctx == NULL) {
		*minor_status = 0;
		return (GSS_S_NO_CONTEXT);
	}

	return (m->gm_wrap_size_limit(minor_status, ctx->gc_ctx,
		    conf_req_flag, qop_req, req_output_size, max_input_size));
}
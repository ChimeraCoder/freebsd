
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

/* $Id$ */

#include "mech_locl.h"

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_pseudo_random(OM_uint32 *minor_status,
		  gss_ctx_id_t context,
		  int prf_key,
		  const gss_buffer_t prf_in,
		  ssize_t desired_output_len,
		  gss_buffer_t prf_out)
{
    struct _gss_context *ctx = (struct _gss_context *) context;
    gssapi_mech_interface m;
    OM_uint32 major_status;

    _mg_buffer_zero(prf_out);
    *minor_status = 0;

    if (ctx == NULL) {
	*minor_status = 0;
	return GSS_S_NO_CONTEXT;
    }

    m = ctx->gc_mech;

    if (m->gm_pseudo_random == NULL)
	return GSS_S_UNAVAILABLE;

    major_status = (*m->gm_pseudo_random)(minor_status, ctx->gc_ctx,
					  prf_key, prf_in, desired_output_len,
					  prf_out);
    if (major_status != GSS_S_COMPLETE)
	_gss_mg_error(m, major_status, *minor_status);

    return major_status;
}
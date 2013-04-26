
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
gss_set_sec_context_option (OM_uint32 *minor_status,
			    gss_ctx_id_t *context_handle,
			    const gss_OID object,
			    const gss_buffer_t value)
{
	struct _gss_context	*ctx;
	OM_uint32		major_status;
	gssapi_mech_interface	m;

	*minor_status = 0;

	if (context_handle == NULL)
		return GSS_S_NO_CONTEXT;

	ctx = (struct _gss_context *) *context_handle;

	if (ctx == NULL)
		return GSS_S_NO_CONTEXT;

	m = ctx->gc_mech;

	if (m == NULL)
		return GSS_S_BAD_MECH;

	if (m->gm_set_sec_context_option != NULL) {
		major_status = m->gm_set_sec_context_option(minor_status,
		    &ctx->gc_ctx, object, value);
		if (major_status != GSS_S_COMPLETE)
			_gss_mg_error(m, major_status, *minor_status);
	} else
		major_status = GSS_S_BAD_MECH;

	return major_status;
}
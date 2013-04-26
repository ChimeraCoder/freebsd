
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
gss_export_sec_context(OM_uint32 *minor_status,
    gss_ctx_id_t *context_handle,
    gss_buffer_t interprocess_token)
{
	OM_uint32 major_status;
	struct _gss_context *ctx = (struct _gss_context *) *context_handle;
	gssapi_mech_interface m = ctx->gc_mech;
	gss_buffer_desc buf;

	_mg_buffer_zero(interprocess_token);

	major_status = m->gm_export_sec_context(minor_status,
	    &ctx->gc_ctx, &buf);

	if (major_status == GSS_S_COMPLETE) {
		unsigned char *p;

		free(ctx);
		*context_handle = GSS_C_NO_CONTEXT;
		interprocess_token->length = buf.length
			+ 2 + m->gm_mech_oid.length;
		interprocess_token->value = malloc(interprocess_token->length);
		if (!interprocess_token->value) {
			/*
			 * We are in trouble here - the context is
			 * already gone. This is allowed as long as we
			 * set the caller's context_handle to
			 * GSS_C_NO_CONTEXT, which we did above.
			 * Return GSS_S_FAILURE.
			 */
			_mg_buffer_zero(interprocess_token);
			*minor_status = ENOMEM;
			return (GSS_S_FAILURE);
		}
		p = interprocess_token->value;
		p[0] = m->gm_mech_oid.length >> 8;
		p[1] = m->gm_mech_oid.length;
		memcpy(p + 2, m->gm_mech_oid.elements, m->gm_mech_oid.length);
		memcpy(p + 2 + m->gm_mech_oid.length, buf.value, buf.length);
		gss_release_buffer(minor_status, &buf);
	} else {
		_gss_mg_error(m, major_status, *minor_status);
	}

	return (major_status);
}
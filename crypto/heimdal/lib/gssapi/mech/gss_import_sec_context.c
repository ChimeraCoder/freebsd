
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
gss_import_sec_context(OM_uint32 *minor_status,
    const gss_buffer_t interprocess_token,
    gss_ctx_id_t *context_handle)
{
	OM_uint32 major_status;
	gssapi_mech_interface m;
	struct _gss_context *ctx;
	gss_OID_desc mech_oid;
	gss_buffer_desc buf;
	unsigned char *p;
	size_t len;

	*minor_status = 0;
	*context_handle = GSS_C_NO_CONTEXT;

	/*
	 * We added an oid to the front of the token in
	 * gss_export_sec_context.
	 */
	p = interprocess_token->value;
	len = interprocess_token->length;
	if (len < 2)
		return (GSS_S_DEFECTIVE_TOKEN);
	mech_oid.length = (p[0] << 8) | p[1];
	if (len < mech_oid.length + 2)
		return (GSS_S_DEFECTIVE_TOKEN);
	mech_oid.elements = p + 2;
	buf.length = len - 2 - mech_oid.length;
	buf.value = p + 2 + mech_oid.length;

	m = __gss_get_mechanism(&mech_oid);
	if (!m)
		return (GSS_S_DEFECTIVE_TOKEN);

	ctx = malloc(sizeof(struct _gss_context));
	if (!ctx) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}
	ctx->gc_mech = m;
	major_status = m->gm_import_sec_context(minor_status,
	    &buf, &ctx->gc_ctx);
	if (major_status != GSS_S_COMPLETE) {
		_gss_mg_error(m, major_status, *minor_status);
		free(ctx);
	} else {
		*context_handle = (gss_ctx_id_t) ctx;
	}

	return (major_status);
}
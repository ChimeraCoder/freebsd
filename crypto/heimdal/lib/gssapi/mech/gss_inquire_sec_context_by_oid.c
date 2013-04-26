
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
gss_inquire_sec_context_by_oid (OM_uint32 *minor_status,
	                        const gss_ctx_id_t context_handle,
	                        const gss_OID desired_object,
	                        gss_buffer_set_t *data_set)
{
	struct _gss_context	*ctx = (struct _gss_context *) context_handle;
	OM_uint32		major_status;
	gssapi_mech_interface	m;

	*minor_status = 0;
	*data_set = GSS_C_NO_BUFFER_SET;
	if (ctx == NULL)
		return GSS_S_NO_CONTEXT;

	/*
	 * select the approprate underlying mechanism routine and
	 * call it.
	 */

	m = ctx->gc_mech;

	if (m == NULL)
		return GSS_S_BAD_MECH;

	if (m->gm_inquire_sec_context_by_oid != NULL) {
		major_status = m->gm_inquire_sec_context_by_oid(minor_status,
		    ctx->gc_ctx, desired_object, data_set);
		if (major_status != GSS_S_COMPLETE)
			_gss_mg_error(m, major_status, *minor_status);
	} else
		major_status = GSS_S_BAD_MECH;

	return major_status;
}
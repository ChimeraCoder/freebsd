
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
gss_set_sec_context_option (OM_uint32 *minor_status,
			    gss_ctx_id_t *context_handle,
			    const gss_OID object,
			    const gss_buffer_t value)
{
	struct _gss_context	*ctx;
	OM_uint32		major_status;
	struct _gss_mech_switch	*m;
	int			one_ok = 0;

	*minor_status = 0;

	if (context_handle == NULL) {
		_gss_load_mech();
		major_status = GSS_S_BAD_MECH;
		SLIST_FOREACH(m, &_gss_mechs, gm_link) {
			if (!m->gm_set_sec_context_option)
				continue;
			major_status = m->gm_set_sec_context_option(
				minor_status,
				NULL, object, value);
			if (major_status == GSS_S_COMPLETE)
				one_ok = 1;
		}
		if (one_ok) {
			*minor_status = 0;
			return (GSS_S_COMPLETE);
		}
		return (major_status);
	}

	ctx = (struct _gss_context *) *context_handle;

	if (ctx == NULL)
		return (GSS_S_NO_CONTEXT);

	m = ctx->gc_mech;

	if (m == NULL)
		return (GSS_S_BAD_MECH);

	if (m->gm_set_sec_context_option != NULL) {
		major_status = m->gm_set_sec_context_option(minor_status,
		    &ctx->gc_ctx, object, value);
		if (major_status != GSS_S_COMPLETE)
			_gss_mg_error(m, major_status, *minor_status);
	} else
		major_status = (GSS_S_BAD_MECH);

	return (major_status);
}
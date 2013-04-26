
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
gss_inquire_context(OM_uint32 *minor_status,
    const gss_ctx_id_t context_handle,
    gss_name_t *src_name,
    gss_name_t *targ_name,
    OM_uint32 *lifetime_rec,
    gss_OID *mech_type,
    OM_uint32 *ctx_flags,
    int *locally_initiated,
    int *xopen)
{
	OM_uint32 major_status;
	struct _gss_context *ctx = (struct _gss_context *) context_handle;
	gssapi_mech_interface m = ctx->gc_mech;
	struct _gss_name *name;
	gss_name_t src_mn, targ_mn;

	if (locally_initiated)
	    *locally_initiated = 0;
	if (xopen)
	    *xopen = 0;
	if (lifetime_rec)
	    *lifetime_rec = 0;

	if (src_name)
	    *src_name = GSS_C_NO_NAME;
	if (targ_name)
	    *targ_name = GSS_C_NO_NAME;
	if (mech_type)
	    *mech_type = GSS_C_NO_OID;
	src_mn = targ_mn = GSS_C_NO_NAME;

	major_status = m->gm_inquire_context(minor_status,
	    ctx->gc_ctx,
	    src_name ? &src_mn : NULL,
	    targ_name ? &targ_mn : NULL,
	    lifetime_rec,
	    mech_type,
	    ctx_flags,
	    locally_initiated,
	    xopen);

	if (major_status != GSS_S_COMPLETE) {
		_gss_mg_error(m, major_status, *minor_status);
		return (major_status);
	}

	if (src_name) {
		name = _gss_make_name(m, src_mn);
		if (!name) {
			if (mech_type)
				*mech_type = GSS_C_NO_OID;
			m->gm_release_name(minor_status, &src_mn);
			*minor_status = 0;
			return (GSS_S_FAILURE);
		}
		*src_name = (gss_name_t) name;
	}

	if (targ_name) {
		name = _gss_make_name(m, targ_mn);
		if (!name) {
			if (mech_type)
				*mech_type = GSS_C_NO_OID;
			if (src_name)
				gss_release_name(minor_status, src_name);
			m->gm_release_name(minor_status, &targ_mn);
			*minor_status = 0;
			return (GSS_S_FAILURE);
		}
		*targ_name = (gss_name_t) name;
	}

	return (GSS_S_COMPLETE);
}
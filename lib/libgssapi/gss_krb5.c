
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
#include <stdlib.h>
#include <errno.h>

#include "mech_switch.h"
#include "context.h"
#include "cred.h"

OM_uint32
gsskrb5_register_acceptor_identity(const char *identity)
{
	struct _gss_mech_switch *m;

	_gss_load_mech();
	SLIST_FOREACH(m, &_gss_mechs, gm_link) {
		if (m->gm_krb5_register_acceptor_identity)
			m->gm_krb5_register_acceptor_identity(identity);
	}

	return (GSS_S_COMPLETE);
}

OM_uint32
gss_krb5_copy_ccache(OM_uint32 *minor_status,
    gss_cred_id_t cred_handle,
    struct krb5_ccache_data *out)
{
	struct _gss_mechanism_cred *mcp;
	struct _gss_cred *cred = (struct _gss_cred *) cred_handle;
	struct _gss_mech_switch *m;

	*minor_status = 0;

	SLIST_FOREACH(mcp, &cred->gc_mc, gmc_link) {
		m = mcp->gmc_mech;
		if (m->gm_krb5_copy_ccache)
			return (m->gm_krb5_copy_ccache(minor_status,
				mcp->gmc_cred, out));
	}

	return (GSS_S_FAILURE);
}

OM_uint32
gss_krb5_compat_des3_mic(OM_uint32 *minor_status,
    gss_ctx_id_t context_handle, int flag)
{
	struct _gss_context *ctx = (struct _gss_context *) context_handle;
	struct _gss_mech_switch *m = ctx->gc_mech;

	*minor_status = 0;

	if (m->gm_krb5_compat_des3_mic)
		return (m->gm_krb5_compat_des3_mic(minor_status,
			ctx->gc_ctx, flag));

	return (GSS_S_FAILURE);
}

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

#include "ntlm.h"

OM_uint32 GSSAPI_CALLCONV _gss_ntlm_acquire_cred
           (OM_uint32 * min_stat,
            const gss_name_t desired_name,
            OM_uint32 time_req,
            const gss_OID_set desired_mechs,
            gss_cred_usage_t cred_usage,
            gss_cred_id_t * output_cred_handle,
            gss_OID_set * actual_mechs,
            OM_uint32 * time_rec
           )
{
    ntlm_name name = (ntlm_name) desired_name;
    OM_uint32 maj_stat;
    ntlm_ctx ctx;

    *min_stat = 0;
    *output_cred_handle = GSS_C_NO_CREDENTIAL;
    if (actual_mechs)
	*actual_mechs = GSS_C_NO_OID_SET;
    if (time_rec)
	*time_rec = GSS_C_INDEFINITE;

    if (desired_name == NULL)
	return GSS_S_NO_CRED;

    if (cred_usage == GSS_C_BOTH || cred_usage == GSS_C_ACCEPT) {

	maj_stat = _gss_ntlm_allocate_ctx(min_stat, &ctx);
	if (maj_stat != GSS_S_COMPLETE)
	    return maj_stat;

	maj_stat = (*ctx->server->nsi_probe)(min_stat, ctx->ictx,
					     name->domain);
	{
	    gss_ctx_id_t context = (gss_ctx_id_t)ctx;
	    OM_uint32 junk;
	    _gss_ntlm_delete_sec_context(&junk, &context, NULL);
	}
	if (maj_stat)
	    return maj_stat;
    }
    if (cred_usage == GSS_C_BOTH || cred_usage == GSS_C_INITIATE) {
	ntlm_cred cred;

	*min_stat = _gss_ntlm_get_user_cred(name, &cred);
	if (*min_stat)
	    return GSS_S_FAILURE;
	cred->usage = cred_usage;

	*output_cred_handle = (gss_cred_id_t)cred;
    }

    return (GSS_S_COMPLETE);
}

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

OM_uint32 GSSAPI_CALLCONV
_gss_ntlm_inquire_context (
            OM_uint32 * minor_status,
            const gss_ctx_id_t context_handle,
            gss_name_t * src_name,
            gss_name_t * targ_name,
            OM_uint32 * lifetime_rec,
            gss_OID * mech_type,
            OM_uint32 * ctx_flags,
            int * locally_initiated,
            int * open_context
           )
{
    ntlm_ctx ctx = (ntlm_ctx)context_handle;

    *minor_status = 0;
    if (src_name)
	*src_name = GSS_C_NO_NAME;
    if (targ_name)
	*targ_name = GSS_C_NO_NAME;
    if (lifetime_rec)
	*lifetime_rec = GSS_C_INDEFINITE;
    if (mech_type)
	*mech_type = GSS_NTLM_MECHANISM;
    if (ctx_flags)
	*ctx_flags = ctx->gssflags;
    if (locally_initiated)
	*locally_initiated = (ctx->status & STATUS_CLIENT) ? 1 : 0;
    if (open_context)
	*open_context = (ctx->status & STATUS_OPEN) ? 1 : 0;

    return GSS_S_COMPLETE;
}

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

#include "gsskrb5_locl.h"

OM_uint32
_gsskrb5_lifetime_left(OM_uint32 *minor_status,
		       krb5_context context,
		       OM_uint32 lifetime,
		       OM_uint32 *lifetime_rec)
{
    krb5_timestamp timeret;
    krb5_error_code kret;

    if (lifetime == 0) {
	*lifetime_rec = GSS_C_INDEFINITE;
	return GSS_S_COMPLETE;
    }

    kret = krb5_timeofday(context, &timeret);
    if (kret) {
	*minor_status = kret;
	return GSS_S_FAILURE;
    }

    if (lifetime < timeret)
	*lifetime_rec = 0;
    else
	*lifetime_rec = lifetime - timeret;

    return GSS_S_COMPLETE;
}


OM_uint32 GSSAPI_CALLCONV _gsskrb5_context_time
           (OM_uint32 * minor_status,
            const gss_ctx_id_t context_handle,
            OM_uint32 * time_rec
           )
{
    krb5_context context;
    OM_uint32 lifetime;
    OM_uint32 major_status;
    const gsskrb5_ctx ctx = (const gsskrb5_ctx) context_handle;

    GSSAPI_KRB5_INIT (&context);

    HEIMDAL_MUTEX_lock(&ctx->ctx_id_mutex);
    lifetime = ctx->lifetime;
    HEIMDAL_MUTEX_unlock(&ctx->ctx_id_mutex);

    major_status = _gsskrb5_lifetime_left(minor_status, context,
					  lifetime, time_rec);
    if (major_status != GSS_S_COMPLETE)
	return major_status;

    *minor_status = 0;

    if (*time_rec == 0)
	return GSS_S_CONTEXT_EXPIRED;

    return GSS_S_COMPLETE;
}
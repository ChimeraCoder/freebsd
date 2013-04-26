
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

#include <roken.h>

#if 0
OM_uint32 GSSAPI_CALLCONV
_gk_wrap_iov(OM_uint32 * minor_status,
	     gss_ctx_id_t  context_handle,
	     int conf_req_flag,
	     gss_qop_t qop_req,
	     int * conf_state,
	     gss_iov_buffer_desc *iov,
	     int iov_count)
{
  const gsskrb5_ctx ctx = (const gsskrb5_ctx) context_handle;
  krb5_context context;

  GSSAPI_KRB5_INIT (&context);

  if (ctx->more_flags & IS_CFX)
      return _gssapi_wrap_cfx_iov(minor_status, ctx, context,
				  conf_req_flag, conf_state,
				  iov, iov_count);

    return GSS_S_FAILURE;
}

OM_uint32 GSSAPI_CALLCONV
_gk_unwrap_iov(OM_uint32 *minor_status,
	       gss_ctx_id_t context_handle,
	       int *conf_state,
	       gss_qop_t *qop_state,
	       gss_iov_buffer_desc *iov,
	       int iov_count)
{
    const gsskrb5_ctx ctx = (const gsskrb5_ctx) context_handle;
    krb5_context context;

    GSSAPI_KRB5_INIT (&context);

    if (ctx->more_flags & IS_CFX)
	return _gssapi_unwrap_cfx_iov(minor_status, ctx, context,
				      conf_state, qop_state, iov, iov_count);

    return GSS_S_FAILURE;
}
#endif

OM_uint32 GSSAPI_CALLCONV
_gk_wrap_iov_length(OM_uint32 * minor_status,
		    gss_ctx_id_t context_handle,
		    int conf_req_flag,
		    gss_qop_t qop_req,
		    int *conf_state,
		    gss_iov_buffer_desc *iov,
		    int iov_count)
{
    const gsskrb5_ctx ctx = (const gsskrb5_ctx) context_handle;
    krb5_context context;

    GSSAPI_KRB5_INIT (&context);

    if (ctx->more_flags & IS_CFX)
	return _gssapi_wrap_iov_length_cfx(minor_status, ctx, context,
					   conf_req_flag, qop_req, conf_state,
					   iov, iov_count);

    return GSS_S_FAILURE;
}
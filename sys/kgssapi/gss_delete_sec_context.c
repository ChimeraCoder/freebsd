
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/kobj.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>

#include <kgssapi/gssapi.h>
#include <kgssapi/gssapi_impl.h>

#include "gssd.h"

OM_uint32
gss_delete_sec_context(OM_uint32 *minor_status, gss_ctx_id_t *context_handle,
	gss_buffer_t output_token)
{
	struct delete_sec_context_res res;
	struct delete_sec_context_args args;
	enum clnt_stat stat;
	gss_ctx_id_t ctx;
	CLIENT *cl;

	*minor_status = 0;

	if (!kgss_gssd_handle)
		return (GSS_S_FAILURE);

	if (*context_handle) {
		ctx = *context_handle;

		/*
		 * If we are past the context establishment phase, let
		 * the in-kernel code do the delete, otherwise
		 * userland needs to deal with it.
		 */
		if (ctx->handle) {
			args.ctx = ctx->handle;
			cl = kgss_gssd_client();
			if (cl == NULL)
				return (GSS_S_FAILURE);
	
			bzero(&res, sizeof(res));
			stat = gssd_delete_sec_context_1(&args, &res, cl);
			CLNT_RELEASE(cl);
			if (stat != RPC_SUCCESS) {
				*minor_status = stat;
				return (GSS_S_FAILURE);
			}

			if (output_token)
				kgss_copy_buffer(&res.output_token,
				    output_token);
			xdr_free((xdrproc_t) xdr_delete_sec_context_res, &res);

			kgss_delete_context(ctx, NULL);
		} else {
			kgss_delete_context(ctx, output_token);
		}
		*context_handle = NULL;
	} else {
		if (output_token) {
			output_token->length = 0;
			output_token->value = NULL;
		}
	}

	return (GSS_S_COMPLETE);
}

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
#include <rpc/rpc.h>

#include "gssd.h"
#include "kgss_if.h"

OM_uint32 gss_accept_sec_context(OM_uint32 *minor_status,
    gss_ctx_id_t *context_handle,
    const gss_cred_id_t acceptor_cred_handle,
    const gss_buffer_t input_token,
    const gss_channel_bindings_t input_chan_bindings,
    gss_name_t *src_name,
    gss_OID *mech_type,
    gss_buffer_t output_token,
    OM_uint32 *ret_flags,
    OM_uint32 *time_rec,
    gss_cred_id_t *delegated_cred_handle)
{
	struct accept_sec_context_res res;
	struct accept_sec_context_args args;
	enum clnt_stat stat;
	gss_ctx_id_t ctx = *context_handle;
	gss_name_t name;
	gss_cred_id_t cred;
	CLIENT *cl;

	cl = kgss_gssd_client();
	if (cl == NULL) {
		*minor_status = 0;
		return (GSS_S_FAILURE);
	}

	if (ctx)
		args.ctx = ctx->handle;
	else
		args.ctx = 0;
	if (acceptor_cred_handle)
		args.cred = acceptor_cred_handle->handle;
	else
		args.cred = 0;
	args.input_token = *input_token;
	args.input_chan_bindings = input_chan_bindings;

	bzero(&res, sizeof(res));
	stat = gssd_accept_sec_context_1(&args, &res, cl);
	CLNT_RELEASE(cl);
	if (stat != RPC_SUCCESS) {
		*minor_status = stat;
		return (GSS_S_FAILURE);
	}

	if (res.major_status != GSS_S_COMPLETE
	    && res.major_status != GSS_S_CONTINUE_NEEDED) {
		*minor_status = res.minor_status;
		xdr_free((xdrproc_t) xdr_accept_sec_context_res, &res);
		return (res.major_status);
	}

	*minor_status = res.minor_status;

	if (!ctx) {
		ctx = kgss_create_context(res.mech_type);
		if (!ctx) {
			xdr_free((xdrproc_t) xdr_accept_sec_context_res, &res);
			*minor_status = 0;
			return (GSS_S_BAD_MECH);
		}
	}
	*context_handle = ctx;

	ctx->handle = res.ctx;
	name = malloc(sizeof(struct _gss_name_t), M_GSSAPI, M_WAITOK);
	name->handle = res.src_name;
	if (src_name) {
		*src_name = name;
	} else {
		OM_uint32 junk;
		gss_release_name(&junk, &name);
	}
	if (mech_type)
		*mech_type = KGSS_MECH_TYPE(ctx);
	kgss_copy_buffer(&res.output_token, output_token);
	if (ret_flags)
		*ret_flags = res.ret_flags;
	if (time_rec)
		*time_rec = res.time_rec;
	cred = malloc(sizeof(struct _gss_cred_id_t), M_GSSAPI, M_WAITOK);
	cred->handle = res.delegated_cred_handle;
	if (delegated_cred_handle) {
		*delegated_cred_handle = cred;
	} else {
		OM_uint32 junk;
		gss_release_cred(&junk, &cred);
	}

	xdr_free((xdrproc_t) xdr_accept_sec_context_res, &res);

	/*
	 * If the context establishment is complete, export it from
	 * userland and hand the result (which includes key material
	 * etc.) to the kernel implementation.
	 */
	if (res.major_status == GSS_S_COMPLETE)
		res.major_status = kgss_transfer_context(ctx);

	return (res.major_status);
}

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
#include <sys/proc.h>

#include <kgssapi/gssapi.h>
#include <kgssapi/gssapi_impl.h>

#include "gssd.h"

OM_uint32
gss_acquire_cred(OM_uint32 *minor_status,
    const gss_name_t desired_name,
    OM_uint32 time_req,
    const gss_OID_set desired_mechs,
    gss_cred_usage_t cred_usage,
    gss_cred_id_t *output_cred_handle,
    gss_OID_set *actual_mechs,
    OM_uint32 *time_rec)
{
	OM_uint32 major_status;
	struct acquire_cred_res res;
	struct acquire_cred_args args;
	enum clnt_stat stat;
	gss_cred_id_t cred;
	int i;
	CLIENT *cl;

	*minor_status = 0;
	cl = kgss_gssd_client();
	if (cl == NULL)
		return (GSS_S_FAILURE);

	args.uid = curthread->td_ucred->cr_uid;
	if (desired_name)
		args.desired_name = desired_name->handle;
	else
		args.desired_name = 0;
	args.time_req = time_req;
	args.desired_mechs = desired_mechs;
	args.cred_usage = cred_usage;

	bzero(&res, sizeof(res));
	stat = gssd_acquire_cred_1(&args, &res, cl);
	CLNT_RELEASE(cl);
	if (stat != RPC_SUCCESS) {
		*minor_status = stat;
		return (GSS_S_FAILURE);
	}

	if (res.major_status != GSS_S_COMPLETE) {
		*minor_status = res.minor_status;
		return (res.major_status);
	}

	cred = malloc(sizeof(struct _gss_cred_id_t), M_GSSAPI, M_WAITOK);
	cred->handle = res.output_cred;
	*output_cred_handle = cred;
	if (actual_mechs) {
		major_status = gss_create_empty_oid_set(minor_status,
		    actual_mechs);
		if (major_status)
			return (major_status);
		for (i = 0; i < res.actual_mechs->count; i++) {
			major_status = gss_add_oid_set_member(minor_status,
			    &res.actual_mechs->elements[i], actual_mechs);
			if (major_status)
				return (major_status);
		}
	}
	if (time_rec)
		*time_rec = res.time_rec;

	xdr_free((xdrproc_t) xdr_acquire_cred_res, &res);

	return (GSS_S_COMPLETE);
}
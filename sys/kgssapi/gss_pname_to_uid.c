
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

#include "kgss_if.h"

OM_uint32
gss_pname_to_uid(OM_uint32 *minor_status, const gss_name_t pname,
    const gss_OID mech, uid_t *uidp)
{
	struct pname_to_uid_res res;
	struct pname_to_uid_args args;
	enum clnt_stat stat;
	CLIENT *cl;

	*minor_status = 0;

	if (pname == GSS_C_NO_NAME)
		return (GSS_S_BAD_NAME);

	cl = kgss_gssd_client();
	if (cl == NULL)
		return (GSS_S_FAILURE);

	args.pname = pname->handle;
	args.mech = mech;

	bzero(&res, sizeof(res));
	stat = gssd_pname_to_uid_1(&args, &res, cl);
	CLNT_RELEASE(cl);
	if (stat != RPC_SUCCESS) {
		*minor_status = stat;
		return (GSS_S_FAILURE);
	}

	if (res.major_status != GSS_S_COMPLETE) {
		*minor_status = res.minor_status;
		return (res.major_status);
	}

	*uidp = res.uid;
	return (GSS_S_COMPLETE);
}

OM_uint32
gss_pname_to_unix_cred(OM_uint32 *minor_status, const gss_name_t pname,
    const gss_OID mech, uid_t *uidp, gid_t *gidp,
    int *numgroups, gid_t *groups)
	      
{
	struct pname_to_uid_res res;
	struct pname_to_uid_args args;
	enum clnt_stat stat;
	int i, n;
	CLIENT *cl;

	*minor_status = 0;

	if (pname == GSS_C_NO_NAME)
		return (GSS_S_BAD_NAME);

	cl = kgss_gssd_client();
	if (cl == NULL)
		return (GSS_S_FAILURE);

	args.pname = pname->handle;
	args.mech = mech;

	bzero(&res, sizeof(res));
	stat = gssd_pname_to_uid_1(&args, &res, cl);
	CLNT_RELEASE(cl);
	if (stat != RPC_SUCCESS) {
		*minor_status = stat;
		return (GSS_S_FAILURE);
	}

	if (res.major_status != GSS_S_COMPLETE) {
		*minor_status = res.minor_status;
		return (res.major_status);
	}

	*uidp = res.uid;
	*gidp = res.gid;
	n = res.gidlist.gidlist_len;
	if (n > *numgroups)
		n = *numgroups;
	for (i = 0; i < n; i++)
		groups[i] = res.gidlist.gidlist_val[i];
	*numgroups = n;

	xdr_free((xdrproc_t) xdr_pname_to_uid_res, &res);

	return (GSS_S_COMPLETE);
}
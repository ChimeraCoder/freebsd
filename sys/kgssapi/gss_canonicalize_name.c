
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
gss_canonicalize_name(OM_uint32 *minor_status,
    gss_name_t input_name,
    const gss_OID mech_type,
    gss_name_t *output_name)
{
	struct canonicalize_name_res res;
	struct canonicalize_name_args args;
	enum clnt_stat stat;
	gss_name_t name;
	CLIENT *cl;

	*minor_status = 0;
	cl = kgss_gssd_client();
	if (cl == NULL)
		return (GSS_S_FAILURE);

	args.input_name = input_name->handle;
	args.mech_type = mech_type;
	
	bzero(&res, sizeof(res));
	stat = gssd_canonicalize_name_1(&args, &res, cl);
	CLNT_RELEASE(cl);
	if (stat != RPC_SUCCESS) {
		*minor_status = stat;
		return (GSS_S_FAILURE);
	}

	if (res.major_status != GSS_S_COMPLETE) {
		*minor_status = res.minor_status;
		return (res.major_status);
	}

	name = malloc(sizeof(struct _gss_name_t), M_GSSAPI, M_WAITOK);
	name->handle = res.output_name;
	*output_name = name;

	return (GSS_S_COMPLETE);
}

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
gss_release_name(OM_uint32 *minor_status, gss_name_t *input_name)
{
	struct release_name_res res;
	struct release_name_args args;
	enum clnt_stat stat;
	gss_name_t name;
	CLIENT *cl;

	*minor_status = 0;

	if (!kgss_gssd_handle)
		return (GSS_S_FAILURE);

	if (*input_name) {
		name = *input_name;
		args.input_name = name->handle;
	
		cl = kgss_gssd_client();
		if (cl == NULL)
			return (GSS_S_FAILURE);
		stat = gssd_release_name_1(&args, &res, cl);
		CLNT_RELEASE(cl);
		if (stat != RPC_SUCCESS) {
			*minor_status = stat;
			return (GSS_S_FAILURE);
		}

		free(name, M_GSSAPI);
		*input_name = NULL;

		if (res.major_status != GSS_S_COMPLETE) {
			*minor_status = res.minor_status;
			return (res.major_status);
		}
	}

	return (GSS_S_COMPLETE);
}
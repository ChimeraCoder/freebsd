
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

#include "kcm_locl.h"

void
kcm_service(void *ctx, const heim_idata *req,
	    const heim_icred cred,
	    heim_ipc_complete complete,
	    heim_sipc_call cctx)
{
    kcm_client peercred;
    krb5_error_code ret;
    krb5_data request, rep;
    unsigned char *buf;
    size_t len;

    krb5_data_zero(&rep);

    peercred.uid = heim_ipc_cred_get_uid(cred);
    peercred.gid = heim_ipc_cred_get_gid(cred);
    peercred.pid = heim_ipc_cred_get_pid(cred);
    peercred.session = heim_ipc_cred_get_session(cred);

    if (req->length < 4) {
	kcm_log(1, "malformed request from process %d (too short)",
		peercred.pid);
	(*complete)(cctx, EINVAL, NULL);
	return;
    }

    buf = req->data;
    len = req->length;

    if (buf[0] != KCM_PROTOCOL_VERSION_MAJOR ||
	buf[1] != KCM_PROTOCOL_VERSION_MINOR) {
	kcm_log(1, "incorrect protocol version %d.%d from process %d",
		buf[0], buf[1], peercred.pid);
	(*complete)(cctx, EINVAL, NULL);
	return;
    }

    request.data = buf + 2;
    request.length = len - 2;

    /* buf is now pointing at opcode */

    ret = kcm_dispatch(kcm_context, &peercred, &request, &rep);

    (*complete)(cctx, ret, &rep);
    krb5_data_free(&rep);
}
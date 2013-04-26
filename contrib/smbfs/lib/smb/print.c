
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
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

/*#include <netnb/netbios.h>*/

#include <netsmb/smb_lib.h>
#include <netsmb/smb_conn.h>
#include <cflib.h>

int
smb_smb_open_print_file(struct smb_ctx *ctx, int setuplen, int mode,
	const char *ident, smbfh *fhp)
{
	struct smb_rq *rqp;
	struct mbdata *mbp;
	int error;

	error = smb_rq_init(ctx, SMB_COM_OPEN_PRINT_FILE, 2, &rqp);
	if (error)
		return error;
	mbp = smb_rq_getrequest(rqp);
	mb_put_uint16le(mbp, setuplen);
	mb_put_uint16le(mbp, mode);
	smb_rq_wend(rqp);
	mb_put_uint8(mbp, SMB_DT_ASCII);
	smb_rq_dstring(mbp, ident);
	error = smb_rq_simple(rqp);
	if (!error) {
		mbp = smb_rq_getreply(rqp);
		mb_get_uint16(mbp, fhp);
	}
	smb_rq_done(rqp);
	return error;
}

int
smb_smb_close_print_file(struct smb_ctx *ctx, smbfh fh)
{
	struct smb_rq *rqp;
	struct mbdata *mbp;
	int error;

	error = smb_rq_init(ctx, SMB_COM_CLOSE_PRINT_FILE, 0, &rqp);
	if (error)
		return error;
	mbp = smb_rq_getrequest(rqp);
	mb_put_mem(mbp, (char*)&fh, 2);
	smb_rq_wend(rqp);
	error = smb_rq_simple(rqp);
	smb_rq_done(rqp);
	return error;
}
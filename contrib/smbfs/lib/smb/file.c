
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

#include <netsmb/smb_lib.h>
#include <netsmb/smb_conn.h>
#include <cflib.h>

int
smb_read(struct smb_ctx *ctx, smbfh fh, off_t offset, size_t count, char *dst)
{
	struct smbioc_rw rwrq;

	rwrq.ioc_fh = fh;
	rwrq.ioc_base = dst;
	rwrq.ioc_cnt = count;
	rwrq.ioc_offset = offset;
	if (ioctl(ctx->ct_fd, SMBIOC_READ, &rwrq) == -1)
		return -1;
	return rwrq.ioc_cnt;
}

int
smb_write(struct smb_ctx *ctx, smbfh fh, off_t offset, size_t count,
	const char *src)
{
	struct smbioc_rw rwrq;

	rwrq.ioc_fh = fh;
	rwrq.ioc_base = (char *)src;
	rwrq.ioc_cnt = count;
	rwrq.ioc_offset = offset;
	if (ioctl(ctx->ct_fd, SMBIOC_WRITE, &rwrq) == -1)
		return -1;
	return rwrq.ioc_cnt;
}
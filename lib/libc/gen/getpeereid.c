
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
#include <sys/socket.h>
#include <sys/ucred.h>
#include <sys/un.h>

#include <errno.h>
#include <unistd.h>

int
getpeereid(int s, uid_t *euid, gid_t *egid)
{
	struct xucred xuc;
	socklen_t xuclen;
	int error;

	xuclen = sizeof(xuc);
	error = getsockopt(s, 0, LOCAL_PEERCRED, &xuc, &xuclen);
	if (error != 0)
		return (error);
	if (xuc.cr_version != XUCRED_VERSION)
		return (EINVAL);
	*euid = xuc.cr_uid;
	*egid = xuc.cr_gid;
	return (0);
}
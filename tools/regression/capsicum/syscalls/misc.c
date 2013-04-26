
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

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>

#include "misc.h"

int
pdwait(int pfd)
{
	fd_set fdset;

	FD_ZERO(&fdset);
	FD_SET(pfd, &fdset);

	return (select(pfd + 1, NULL, &fdset, NULL, NULL) == -1 ? -1 : 0);
}

int
descriptor_send(int sock, int fd)
{
	unsigned char ctrl[CMSG_SPACE(sizeof(fd))];
	struct msghdr msg;
	struct cmsghdr *cmsg;

	assert(sock >= 0);
	assert(fd >= 0);

	bzero(&msg, sizeof(msg));
	bzero(&ctrl, sizeof(ctrl));

	msg.msg_iov = NULL;
	msg.msg_iovlen = 0;
	msg.msg_control = ctrl;
	msg.msg_controllen = sizeof(ctrl);

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	bcopy(&fd, CMSG_DATA(cmsg), sizeof(fd));

	if (sendmsg(sock, &msg, 0) == -1)
		return (errno);

	return (0);
}

int
descriptor_recv(int sock, int *fdp)
{
	unsigned char ctrl[CMSG_SPACE(sizeof(*fdp))];
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov;
	int val;

	assert(sock >= 0);
	assert(fdp != NULL);

	bzero(&msg, sizeof(msg));
	bzero(&ctrl, sizeof(ctrl));

#if 1
	/*
	 * This doesn't really make sense, as we don't plan to receive any
	 * data, but if no buffer is provided and recv(2) returns 0 without
	 * control message. Must be kernel bug.
	 */
	iov.iov_base = &val;
	iov.iov_len = sizeof(val);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
#else
	msg.msg_iov = NULL;
	msg.msg_iovlen = 0;
#endif
	msg.msg_control = ctrl;
	msg.msg_controllen = sizeof(ctrl);

	if (recvmsg(sock, &msg, 0) == -1)
		return (errno);

	cmsg = CMSG_FIRSTHDR(&msg);
	if (cmsg == NULL || cmsg->cmsg_level != SOL_SOCKET ||
	    cmsg->cmsg_type != SCM_RIGHTS) {
		return (EINVAL);
	}
	bcopy(CMSG_DATA(cmsg), fdp, sizeof(*fdp));

	return (0);
}
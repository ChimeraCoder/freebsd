
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

#include "includes.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#include <errno.h>
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#include <string.h>
#include <stdarg.h>

#include "log.h"
#include "monitor_fdpass.h"

int
mm_send_fd(int sock, int fd)
{
#if defined(HAVE_SENDMSG) && (defined(HAVE_ACCRIGHTS_IN_MSGHDR) || defined(HAVE_CONTROL_IN_MSGHDR))
	struct msghdr msg;
#ifndef HAVE_ACCRIGHTS_IN_MSGHDR
	union {
		struct cmsghdr hdr;
		char buf[CMSG_SPACE(sizeof(int))];
	} cmsgbuf;
	struct cmsghdr *cmsg;
#endif
	struct iovec vec;
	char ch = '\0';
	ssize_t n;
	struct pollfd pfd;

	memset(&msg, 0, sizeof(msg));
#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof(fd);
#else
	msg.msg_control = (caddr_t)&cmsgbuf.buf;
	msg.msg_controllen = sizeof(cmsgbuf.buf);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(cmsg) = fd;
#endif

	vec.iov_base = &ch;
	vec.iov_len = 1;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;

	pfd.fd = sock;
	pfd.events = POLLOUT;
	while ((n = sendmsg(sock, &msg, 0)) == -1 &&
	    (errno == EAGAIN || errno == EINTR)) {
		debug3("%s: sendmsg(%d): %s", __func__, fd, strerror(errno));
		(void)poll(&pfd, 1, -1);
	}
	if (n == -1) {
		error("%s: sendmsg(%d): %s", __func__, fd,
		    strerror(errno));
		return -1;
	}

	if (n != 1) {
		error("%s: sendmsg: expected sent 1 got %ld",
		    __func__, (long)n);
		return -1;
	}
	return 0;
#else
	error("%s: file descriptor passing not supported", __func__);
	return -1;
#endif
}

int
mm_receive_fd(int sock)
{
#if defined(HAVE_RECVMSG) && (defined(HAVE_ACCRIGHTS_IN_MSGHDR) || defined(HAVE_CONTROL_IN_MSGHDR))
	struct msghdr msg;
#ifndef HAVE_ACCRIGHTS_IN_MSGHDR
	union {
		struct cmsghdr hdr;
		char buf[CMSG_SPACE(sizeof(int))];
	} cmsgbuf;
	struct cmsghdr *cmsg;
#endif
	struct iovec vec;
	ssize_t n;
	char ch;
	int fd;
	struct pollfd pfd;

	memset(&msg, 0, sizeof(msg));
	vec.iov_base = &ch;
	vec.iov_len = 1;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
	msg.msg_accrights = (caddr_t)&fd;
	msg.msg_accrightslen = sizeof(fd);
#else
	msg.msg_control = &cmsgbuf.buf;
	msg.msg_controllen = sizeof(cmsgbuf.buf);
#endif

	pfd.fd = sock;
	pfd.events = POLLIN;
	while ((n = recvmsg(sock, &msg, 0)) == -1 &&
	    (errno == EAGAIN || errno == EINTR)) {
		debug3("%s: recvmsg: %s", __func__, strerror(errno));
		(void)poll(&pfd, 1, -1);
	}
	if (n == -1) {
		error("%s: recvmsg: %s", __func__, strerror(errno));
		return -1;
	}

	if (n != 1) {
		error("%s: recvmsg: expected received 1 got %ld",
		    __func__, (long)n);
		return -1;
	}

#ifdef HAVE_ACCRIGHTS_IN_MSGHDR
	if (msg.msg_accrightslen != sizeof(fd)) {
		error("%s: no fd", __func__);
		return -1;
	}
#else
	cmsg = CMSG_FIRSTHDR(&msg);
	if (cmsg == NULL) {
		error("%s: no message header", __func__);
		return -1;
	}

#ifndef BROKEN_CMSG_TYPE
	if (cmsg->cmsg_type != SCM_RIGHTS) {
		error("%s: expected type %d got %d", __func__,
		    SCM_RIGHTS, cmsg->cmsg_type);
		return -1;
	}
#endif
	fd = (*(int *)CMSG_DATA(cmsg));
#endif
	return fd;
#else
	error("%s: file descriptor passing not supported", __func__);
	return -1;
#endif
}

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
#include <sys/capability.h>
#include <sys/procdesc.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "misc.h"

static void
fcntl_tests_0(int fd)
{
	uint32_t fcntlrights;

	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_ALL);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
	CHECK(fcntl(fd, F_GETFL) == (O_RDWR | O_NONBLOCK));
	CHECK(fcntl(fd, F_SETFL, 0) == 0);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);

	errno = 0;
	CHECK(cap_fcntls_limit(fd, ~CAP_FCNTL_ALL) == -1);
	CHECK(errno == EINVAL);
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == 0);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == (CAP_FCNTL_GETFL | CAP_FCNTL_SETFL));
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == 0);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == (CAP_FCNTL_GETFL | CAP_FCNTL_SETFL));

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
	CHECK(fcntl(fd, F_GETFL) == (O_RDWR | O_NONBLOCK));
	CHECK(fcntl(fd, F_SETFL, 0) == 0);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);

	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == 0);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_GETFL);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_GETFL);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == -1);
	CHECK(errno == ENOTCAPABLE);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, 0) == -1);
	CHECK(errno == ENOTCAPABLE);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);

	CHECK(cap_fcntls_limit(fd, 0) == 0);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, 0) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
}

static void
fcntl_tests_1(int fd)
{
	uint32_t fcntlrights;

	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == 0);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_GETFL);

	CHECK(cap_rights_limit(fd, CAP_ALL & ~CAP_FCNTL) == 0);

	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);

	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, 0) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
}

static void
fcntl_tests_2(int fd)
{
	uint32_t fcntlrights;

	CHECK(cap_rights_limit(fd, CAP_ALL & ~CAP_FCNTL) == 0);

	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);

	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = CAP_FCNTL_ALL;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, 0) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
}

static void
fcntl_tests_send_0(int sock)
{
	int fd;

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK(descriptor_send(sock, fd) == 0);
	CHECK(close(fd) == 0);

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == 0);
	CHECK(descriptor_send(sock, fd) == 0);
	CHECK(close(fd) == 0);

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == 0);
	CHECK(descriptor_send(sock, fd) == 0);
	CHECK(close(fd) == 0);

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK(cap_fcntls_limit(fd, 0) == 0);
	CHECK(descriptor_send(sock, fd) == 0);
	CHECK(close(fd) == 0);
}

static void
fcntl_tests_recv_0(int sock)
{
	uint32_t fcntlrights;
	int fd;

	CHECK(descriptor_recv(sock, &fd) == 0);

	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_ALL);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
	CHECK(fcntl(fd, F_GETFL) == (O_RDWR | O_NONBLOCK));
	CHECK(fcntl(fd, F_SETFL, 0) == 0);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);

	CHECK(close(fd) == 0);

	CHECK(descriptor_recv(sock, &fd) == 0);

	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == (CAP_FCNTL_GETFL | CAP_FCNTL_SETFL));
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == 0);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == (CAP_FCNTL_GETFL | CAP_FCNTL_SETFL));

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == 0);
	CHECK(fcntl(fd, F_GETFL) == (O_RDWR | O_NONBLOCK));
	CHECK(fcntl(fd, F_SETFL, 0) == 0);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);

	CHECK(close(fd) == 0);

	CHECK(descriptor_recv(sock, &fd) == 0);

	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_GETFL);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_GETFL);
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == 0);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == CAP_FCNTL_GETFL);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == -1);
	CHECK(errno == ENOTCAPABLE);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, 0) == -1);
	CHECK(errno == ENOTCAPABLE);
	CHECK(fcntl(fd, F_GETFL) == O_RDWR);

	CHECK(close(fd) == 0);

	CHECK(descriptor_recv(sock, &fd) == 0);

	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL | CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);
	errno = 0;
	CHECK(cap_fcntls_limit(fd, CAP_FCNTL_SETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	fcntlrights = 0;
	CHECK(cap_fcntls_get(fd, &fcntlrights) == 0);
	CHECK(fcntlrights == 0);

	CHECK(fcntl(fd, F_GETFD) == 0);
	CHECK(fcntl(fd, F_SETFD, FD_CLOEXEC) == 0);
	CHECK(fcntl(fd, F_GETFD) == FD_CLOEXEC);
	CHECK(fcntl(fd, F_SETFD, 0) == 0);
	CHECK(fcntl(fd, F_GETFD) == 0);

	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, O_NONBLOCK) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_SETFL, 0) == -1);
	CHECK(errno == ENOTCAPABLE);
	errno = 0;
	CHECK(fcntl(fd, F_GETFL) == -1);
	CHECK(errno == ENOTCAPABLE);

	CHECK(close(fd) == 0);
}

int
main(void)
{
	int fd, pfd, sp[2];
	pid_t pid;

	printf("1..870\n");

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	fcntl_tests_0(fd);
	CHECK(close(fd) == 0);

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	fcntl_tests_1(fd);
	CHECK(close(fd) == 0);

	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	fcntl_tests_2(fd);
	CHECK(close(fd) == 0);

	/* Child inherits descriptor and operates on it first. */
	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK((pid = fork()) >= 0);
	if (pid == 0) {
		fcntl_tests_0(fd);
		CHECK(close(fd) == 0);
		exit(0);
	} else {
		CHECK(waitpid(pid, NULL, 0) == pid);
		fcntl_tests_0(fd);
	}
	CHECK(close(fd) == 0);

	/* Child inherits descriptor, but operates on it after parent. */
	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK((pid = fork()) >= 0);
	if (pid == 0) {
		sleep(1);
		fcntl_tests_0(fd);
		CHECK(close(fd) == 0);
		exit(0);
	} else {
		fcntl_tests_0(fd);
		CHECK(waitpid(pid, NULL, 0) == pid);
	}
	CHECK(close(fd) == 0);

	/* Child inherits descriptor and operates on it first. */
	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK((pid = pdfork(&pfd, 0)) >= 0);
	if (pid == 0) {
		fcntl_tests_1(fd);
		exit(0);
	} else {
		CHECK(pdwait(pfd) == 0);
/*
		It fails with EBADF, which I believe is a bug.
		CHECK(close(pfd) == 0);
*/
		fcntl_tests_1(fd);
	}
	CHECK(close(fd) == 0);

	/* Child inherits descriptor, but operates on it after parent. */
	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK((pid = pdfork(&pfd, 0)) >= 0);
	if (pid == 0) {
		sleep(1);
		fcntl_tests_1(fd);
		exit(0);
	} else {
		fcntl_tests_1(fd);
		CHECK(pdwait(pfd) == 0);
/*
		It fails with EBADF, which I believe is a bug.
		CHECK(close(pfd) == 0);
*/
	}
	CHECK(close(fd) == 0);

	/* Child inherits descriptor and operates on it first. */
	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK((pid = fork()) >= 0);
	if (pid == 0) {
		fcntl_tests_2(fd);
		exit(0);
	} else {
		CHECK(waitpid(pid, NULL, 0) == pid);
		fcntl_tests_2(fd);
	}
	CHECK(close(fd) == 0);

	/* Child inherits descriptor, but operates on it after parent. */
	CHECK((fd = socket(AF_UNIX, SOCK_STREAM, 0)) >= 0);
	CHECK((pid = fork()) >= 0);
	if (pid == 0) {
		sleep(1);
		fcntl_tests_2(fd);
		exit(0);
	} else {
		fcntl_tests_2(fd);
		CHECK(waitpid(pid, NULL, 0) == pid);
	}
	CHECK(close(fd) == 0);

	/* Send descriptors from parent to child. */
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
	CHECK((pid = fork()) >= 0);
	if (pid == 0) {
		CHECK(close(sp[0]) == 0);
		fcntl_tests_recv_0(sp[1]);
		CHECK(close(sp[1]) == 0);
		exit(0);
	} else {
		CHECK(close(sp[1]) == 0);
		fcntl_tests_send_0(sp[0]);
		CHECK(waitpid(pid, NULL, 0) == pid);
		CHECK(close(sp[0]) == 0);
	}

	/* Send descriptors from child to parent. */
	CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sp) == 0);
	CHECK((pid = fork()) >= 0);
	if (pid == 0) {
		CHECK(close(sp[0]) == 0);
		fcntl_tests_send_0(sp[1]);
		CHECK(close(sp[1]) == 0);
		exit(0);
	} else {
		CHECK(close(sp[1]) == 0);
		fcntl_tests_recv_0(sp[0]);
		CHECK(waitpid(pid, NULL, 0) == pid);
		CHECK(close(sp[0]) == 0);
	}

	exit(0);
}

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

#include <sys/types.h>
#include <sys/socket.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Open, then close a set of UNIX domain socket pairs for datagram and
 * stream.
 *
 * Confirm that we can't open INET datagram or stream socket pairs.
 *
 * More tests should be added, including confirming that sending on either
 * endpoint results in data at the other, that the right kind of socket was
 * created (stream vs. datagram), and that message boundaries fall in the
 * right places.
 */
int
main(int argc, char *argv[])
{
	int fd1, fd2, fd3;
	int sv[2];

	/*
	 * UNIX domain socket pair, datagram.
	 */
	if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sv) != 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_DGRAM): %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (close(sv[0]) != 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_DGRAM) close 0: %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (close(sv[1]) != 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_DGRAM) close 1: %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}

	/*
	 * UNIX domain socket pair, stream.
	 */
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) != 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_STREAM): %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (close(sv[0]) != 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_STREAM) close 0: %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (close(sv[1]) != 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_STREAM) close 1: "
		    "%s\n", strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}

	/*
	 * Confirm that PF_INET datagram socket pair creation fails.
	 */
	if (socketpair(PF_INET, SOCK_DGRAM, 0, sv) == 0) {
		fprintf(stderr, "socketpair(PF_INET, SOCK_DGRAM): opened\n");
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (errno != EOPNOTSUPP) {
		fprintf(stderr, "socketpair(PF_INET, SOCK_DGRAM): %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
	}

	/*
	 * Confirm that PF_INET stream socket pair creation fails.
	 */
	if (socketpair(PF_INET, SOCK_STREAM, 0, sv) == 0) {
		fprintf(stderr, "socketpair(PF_INET, SOCK_STREAM): opened\n");
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (errno != EOPNOTSUPP) {
		fprintf(stderr, "socketpair(PF_INET, SOCK_STREAM): %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
	}

	/*
	 * Check for sequential fd allocation, and give up early if not.
	 */
	fd1 = dup(STDIN_FILENO);
	fd2 = dup(STDIN_FILENO);
	if (fd2 != fd1 + 1) {
		fprintf(stderr, "Non-sequential fd allocation\n");
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}

	/* Allocate a socketpair using a bad destination address. */
	if (socketpair(PF_UNIX, SOCK_DGRAM, 0, NULL) == 0) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_DGRAM, NULL): opened\n");
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}
	if (errno != EFAULT) {
		fprintf(stderr, "socketpair(PF_UNIX, SOCK_DGRAM, NULL): %s\n",
		    strerror(errno));
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}

	/* Allocate a file descriptor and make sure it's fd2+1. */
	fd3 = dup(STDIN_FILENO);
	if (fd3 != fd2 + 1) {
		fprintf(stderr, "socketpair(..., NULL) allocated descriptors\n");
		fprintf(stderr, "FAIL\n");
		exit(-1);
	}

	fprintf(stderr, "PASS\n");
	exit(0);
}
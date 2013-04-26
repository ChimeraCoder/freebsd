
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "inout.h"
#include "dbgport.h"

#define	BVM_DBG_PORT	0x224
#define	BVM_DBG_SIG	('B' << 8 | 'V')

static int listen_fd, conn_fd;

static struct sockaddr_in sin;

static int
dbg_handler(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
	    uint32_t *eax, void *arg)
{
	char ch;
	int nwritten, nread, printonce;

	if (bytes == 2 && in) {
		*eax = BVM_DBG_SIG;
		return (0);
	}

	if (bytes != 4)
		return (-1);

again:
	printonce = 0;
	while (conn_fd < 0) {
		if (!printonce) {
			printf("Waiting for connection from gdb\r\n");
			printonce = 1;
		}
		conn_fd = accept(listen_fd, NULL, NULL);
		if (conn_fd >= 0)
			fcntl(conn_fd, F_SETFL, O_NONBLOCK);
		else if (errno != EINTR)
			perror("accept");
	}

	if (in) {
		nread = read(conn_fd, &ch, 1);
		if (nread == -1 && errno == EAGAIN)
			*eax = -1;
		else if (nread == 1)
			*eax = ch;
		else {
			close(conn_fd);
			conn_fd = -1;
			goto again;
		}
	} else {
		ch = *eax;
		nwritten = write(conn_fd, &ch, 1);
		if (nwritten != 1) {
			close(conn_fd);
			conn_fd = -1;
			goto again;
		}
	}
	return (0);
}

static struct inout_port dbgport = {
	"bvmdbg",
	BVM_DBG_PORT,
	1,
	IOPORT_F_INOUT,
	dbg_handler
};

void
init_dbgport(int sport)
{
	conn_fd = -1;

	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	sin.sin_len = sizeof(sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(sport);

	if (bind(listen_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("bind");
		exit(1);
	}

	if (listen(listen_fd, 1) < 0) {
		perror("listen");
		exit(1);
	}

	register_inout(&dbgport);
}

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

/*
 * The reconnect regression test is designed to catch kernel bug that may
 * prevent changing association of already associated datagram unix domain
 * socket when server side of connection has been closed.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

static char *uds_name1 = NULL;
static char *uds_name2 = NULL;

#define	sstosa(ss)	((struct sockaddr *)(ss))

void
prepare_ifsun(struct sockaddr_un *ifsun, const char *path)
{

    memset(ifsun, '\0', sizeof(*ifsun));
#if !defined(__linux__) && !defined(__solaris__)
    ifsun->sun_len = strlen(path);
#endif
    ifsun->sun_family = AF_LOCAL;
    strcpy(ifsun->sun_path, path);
}

int
create_uds_server(const char *path)
{
    struct sockaddr_un ifsun;
    int sock;

    prepare_ifsun(&ifsun, path);

    unlink(ifsun.sun_path);

    sock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sock == -1)
        err(1, "can't create socket");
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sock, sizeof(sock));
    if (bind(sock, sstosa(&ifsun), sizeof(ifsun)) < 0)
        err(1, "can't bind to a socket");

    return sock;
}

void
connect_uds_server(int sock, const char *path)
{
    struct sockaddr_un ifsun;
    int e;

    prepare_ifsun(&ifsun, path);

    e = connect(sock, sstosa(&ifsun), sizeof(ifsun));
    if (e < 0)
        err(1, "can't connect to a socket");
}

void
cleanup(void)
{

    if (uds_name1 != NULL)
        unlink(uds_name1);
    if (uds_name2 != NULL)
        unlink(uds_name2);
}

int
main()
{
    int s_sock1, s_sock2, c_sock;

    atexit(cleanup);

    uds_name1 = strdup("/tmp/reconnect.XXXXXX");
    if (uds_name1 == NULL)
        err(1, "can't allocate memory");
    uds_name1 = mktemp(uds_name1);
    if (uds_name1 == NULL)
        err(1, "mktemp(3) failed");
    s_sock1 = create_uds_server(uds_name1);

    uds_name2 = strdup("/tmp/reconnect.XXXXXX");
    if (uds_name2 == NULL)
        err(1, "can't allocate memory");
    uds_name2 = mktemp(uds_name2);
    if (uds_name2 == NULL)
        err(1, "mktemp(3) failed");
    s_sock2 = create_uds_server(uds_name2);

    c_sock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (c_sock < 0)
        err(1, "can't create socket");

    connect_uds_server(c_sock, uds_name1);
    close(s_sock1);
    connect_uds_server(c_sock, uds_name2);

    exit (0);
}
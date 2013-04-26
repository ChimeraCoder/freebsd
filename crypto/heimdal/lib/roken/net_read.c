
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

#include <config.h>

#include "roken.h"

/*
 * Like read but never return partial data.
 */

#ifndef _WIN32

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
net_read (rk_socket_t fd, void *buf, size_t nbytes)
{
    char *cbuf = (char *)buf;
    ssize_t count;
    size_t rem = nbytes;

    while (rem > 0) {
	count = read (fd, cbuf, rem);
	if (count < 0) {
	    if (errno == EINTR)
		continue;
	    else
		return count;
	} else if (count == 0) {
	    return count;
	}
	cbuf += count;
	rem -= count;
    }
    return nbytes;
}

#else

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
net_read(rk_socket_t sock, void *buf, size_t nbytes)
{
    char *cbuf = (char *)buf;
    ssize_t count;
    size_t rem = nbytes;

#ifdef SOCKET_IS_NOT_AN_FD
    int use_read = 0;
#endif

    while (rem > 0) {
#ifdef SOCKET_IS_NOT_AN_FD
	if (use_read)
	    count = _read (sock, cbuf, rem);
	else
	    count = recv (sock, cbuf, rem, 0);

	if (use_read == 0 &&
	    rk_IS_SOCKET_ERROR(count) &&
            (rk_SOCK_ERRNO == WSANOTINITIALISED ||
             rk_SOCK_ERRNO == WSAENOTSOCK)) {
	    use_read = 1;

	    count = _read (sock, cbuf, rem);
	}
#else
	count = recv (sock, cbuf, rem, 0);
#endif
	if (count < 0) {

	    /* With WinSock, the error EINTR (WSAEINTR), is used to
	       indicate that a blocking call was cancelled using
	       WSACancelBlockingCall(). */

#ifndef HAVE_WINSOCK
	    if (rk_SOCK_ERRNO == EINTR)
		continue;
#endif
	    return count;
	} else if (count == 0) {
	    return count;
	}
	cbuf += count;
	rem -= count;
    }
    return nbytes;
}

#endif
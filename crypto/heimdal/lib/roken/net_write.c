
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
 * Like write but never return partial data.
 */

#ifndef _WIN32

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
net_write (rk_socket_t fd, const void *buf, size_t nbytes)
{
    const char *cbuf = (const char *)buf;
    ssize_t count;
    size_t rem = nbytes;

    while (rem > 0) {
	count = write (fd, cbuf, rem);
	if (count < 0) {
	    if (errno == EINTR)
		continue;
	    else
		return count;
	}
	cbuf += count;
	rem -= count;
    }
    return nbytes;
}

#else

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
net_write(rk_socket_t sock, const void *buf, size_t nbytes)
{
    const char *cbuf = (const char *)buf;
    ssize_t count;
    size_t rem = nbytes;
#ifdef SOCKET_IS_NOT_AN_FD
    int use_write = 0;
#endif

    while (rem > 0) {
#ifdef SOCKET_IS_NOT_AN_FD
	if (use_write)
	    count = _write (sock, cbuf, rem);
	else
	    count = send (sock, cbuf, rem, 0);

	if (use_write == 0 &&
	    rk_IS_SOCKET_ERROR(count) &&
	    (rk_SOCK_ERRNO == WSANOTINITIALISED ||
             rk_SOCK_ERRNO == WSAENOTSOCK)) {
	    use_write = 1;

	    count = _write (sock, cbuf, rem);
	}
#else
	count = send (sock, cbuf, rem, 0);
#endif
	if (count < 0) {
	    if (errno == EINTR)
		continue;
	    else
		return count;
	}
	cbuf += count;
	rem -= count;
    }
    return nbytes;
}

#endif
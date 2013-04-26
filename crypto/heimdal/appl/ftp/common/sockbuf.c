
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

#include "common.h"
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

RCSID("$Id$");

void
set_buffer_size(int fd, int read)
{
#if defined(SO_RCVBUF) && defined(SO_SNDBUF) && defined(HAVE_SETSOCKOPT)
    int size = 4194304;
    int optname = read ? SO_RCVBUF : SO_SNDBUF;

#ifdef HAVE_GETSOCKOPT
    int curr=0;
    socklen_t optlen;

    optlen = sizeof(curr);
    if(getsockopt(fd, SOL_SOCKET, optname, (void *)&curr, &optlen) == 0) {
        if(curr >= size) {
            /* Already large enough */
            return;
        }
    }
#endif /* HAVE_GETSOCKOPT */

    while(size >= 131072 &&
	  setsockopt(fd, SOL_SOCKET, optname, (void *)&size, sizeof(size)) < 0)
	size /= 2;
#endif
}
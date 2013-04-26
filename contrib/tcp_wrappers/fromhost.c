
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
  * On socket-only systems, fromhost() is nothing but an alias for the
  * socket-specific sock_host() function.
  * 
  * On systems with sockets and TLI, fromhost() determines the type of API
  * (sockets, TLI), then invokes the appropriate API-specific routines.
  * 
  * Diagnostics are reported through syslog(3).
  * 
  * Author: Wietse Venema, Eindhoven University of Technology, The Netherlands.
  */
#ifndef lint
static char sccsid[] = "@(#) fromhost.c 1.17 94/12/28 17:42:23";
#endif

#if defined(TLI) || defined(PTX) || defined(TLI_SEQUENT)

/* System libraries. */

#include <sys/types.h>
#include <sys/tiuser.h>
#include <stropts.h>

/* Local stuff. */

#include "tcpd.h"

/* fromhost - find out what network API we should use */

void    fromhost(request)
struct request_info *request;
{

    /*
     * On systems with streams support the IP network protocol family may be
     * accessible via more than one programming interface: Berkeley sockets
     * and the Transport Level Interface (TLI).
     * 
     * Thus, we must first find out what programming interface to use: sockets
     * or TLI. On some systems, sockets are not part of the streams system,
     * so if request->fd is not a stream we simply assume sockets.
     */

    if (ioctl(request->fd, I_FIND, "timod") > 0) {
	tli_host(request);
    } else {
	sock_host(request);
    }
}

#endif /* TLI || PTX || TLI_SEQUENT */
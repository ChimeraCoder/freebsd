
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
 * socktoa - return a numeric host name from a sockaddr_storage structure
 */#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"
#include "ntp.h"


char *
socktohost(
	struct sockaddr_storage* sock
	)
{
	register char *buffer;

	LIB_GETBUF(buffer);
	if (getnameinfo((struct sockaddr *)sock, SOCKLEN(sock), buffer,
	    LIB_BUFLENGTH /* NI_MAXHOST*/, NULL, 0, 0))
		return stoa(sock);

  	return buffer;
}
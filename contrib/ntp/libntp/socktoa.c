
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
 */
#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#ifdef ISC_PLATFORM_NEEDNTOP
#include <isc/net.h>
#endif

#include <stdio.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"
#include "ntp.h"

char *
socktoa(
	struct sockaddr_storage* sock
	)
{
	register char *buffer;

	LIB_GETBUF(buffer);

	if (sock == NULL)
		strcpy(buffer, "null");
	else
	{

		switch(sock->ss_family) {

		default:
		case AF_INET :
			inet_ntop(AF_INET, &GET_INADDR(*sock), buffer,
			    LIB_BUFLENGTH);
			break;

		case AF_INET6 :
			inet_ntop(AF_INET6, &GET_INADDR6(*sock), buffer,
			    LIB_BUFLENGTH);
#if 0
		default:
			strcpy(buffer, "unknown");
#endif
		}
	}
  	return buffer;
}
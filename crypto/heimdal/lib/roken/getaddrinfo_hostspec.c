
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

/* getaddrinfo via string specifying host and port */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
roken_getaddrinfo_hostspec2(const char *hostspec,
			    int socktype,
			    int port,
			    struct addrinfo **ai)
{
    const char *p;
    char portstr[NI_MAXSERV];
    char host[MAXHOSTNAMELEN];
    struct addrinfo hints;
    int hostspec_len;

    struct hst {
	const char *prefix;
	int socktype;
	int protocol;
	int port;
    } *hstp, hst[] = {
	{ "http://", SOCK_STREAM, IPPROTO_TCP, 80 },
	{ "http/", SOCK_STREAM, IPPROTO_TCP, 80 },
	{ "tcp/", SOCK_STREAM, IPPROTO_TCP, 0 },
	{ "udp/", SOCK_DGRAM, IPPROTO_UDP, 0 },
	{ NULL, 0, 0, 0 }
    };

    memset(&hints, 0, sizeof(hints));

    hints.ai_socktype = socktype;

    for(hstp = hst; hstp->prefix; hstp++) {
	if(strncmp(hostspec, hstp->prefix, strlen(hstp->prefix)) == 0) {
	    hints.ai_socktype = hstp->socktype;
	    hints.ai_protocol = hstp->protocol;
	    if(port == 0)
		port = hstp->port;
	    hostspec += strlen(hstp->prefix);
	    break;
	}
    }

    p = strchr (hostspec, ':');
    if (p != NULL) {
	char *end;

	port = strtol (p + 1, &end, 0);
	hostspec_len = p - hostspec;
    } else {
	hostspec_len = strlen(hostspec);
    }
    snprintf (portstr, sizeof(portstr), "%u", port);

    snprintf (host, sizeof(host), "%.*s", hostspec_len, hostspec);
    return getaddrinfo (host, portstr, &hints, ai);
}

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
roken_getaddrinfo_hostspec(const char *hostspec,
			   int port,
			   struct addrinfo **ai)
{
    return roken_getaddrinfo_hostspec2(hostspec, 0, port, ai);
}
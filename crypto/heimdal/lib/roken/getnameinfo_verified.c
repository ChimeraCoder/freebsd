
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
 * Try to obtain a verified name for the address in `sa, salen' (much
 * similar to getnameinfo).
 * Verified in this context means that forwards and backwards lookups
 * in DNS are consistent.  If that fails, return an error if the
 * NI_NAMEREQD flag is set or return the numeric address as a string.
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
getnameinfo_verified(const struct sockaddr *sa, socklen_t salen,
		     char *host, size_t hostlen,
		     char *serv, size_t servlen,
		     int flags)
{
    int ret;
    struct addrinfo *ai, *a;
    char servbuf[NI_MAXSERV];
    struct addrinfo hints;
    void *saaddr;
    size_t sasize;

    if (host == NULL)
	return EAI_NONAME;

    if (serv == NULL) {
	serv = servbuf;
	servlen = sizeof(servbuf);
    }

    ret = getnameinfo (sa, salen, host, hostlen, serv, servlen,
		       flags | NI_NUMERICSERV);
    if (ret)
	goto fail;

    memset (&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    ret = getaddrinfo (host, serv, &hints, &ai);
    if (ret)
	goto fail;

    saaddr = socket_get_address(sa);
    sasize = socket_addr_size(sa);
    for (a = ai; a != NULL; a = a->ai_next) {
	if (sasize == socket_addr_size(a->ai_addr) &&
	    memcmp(saaddr, socket_get_address(a->ai_addr), sasize) == 0) {
	    freeaddrinfo (ai);
	    return 0;
	}
    }
    freeaddrinfo (ai);
 fail:
    if (flags & NI_NAMEREQD)
	return EAI_NONAME;
    ret = getnameinfo (sa, salen, host, hostlen, serv, servlen,
		       flags | NI_NUMERICSERV | NI_NUMERICHOST);
    return ret;
}
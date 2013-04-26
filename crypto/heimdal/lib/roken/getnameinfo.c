
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

static int
doit (int af,
      const void *addr,
      size_t addrlen,
      int port,
      char *host, size_t hostlen,
      char *serv, size_t servlen,
      int flags)
{
    if (host != NULL) {
	if (flags & NI_NUMERICHOST) {
	    if (inet_ntop (af, addr, host, hostlen) == NULL)
		return EAI_SYSTEM;
	} else {
	    struct hostent *he = gethostbyaddr (addr,
						addrlen,
						af);
	    if (he != NULL) {
		strlcpy (host, hostent_find_fqdn(he), hostlen);
		if (flags & NI_NOFQDN) {
		    char *dot = strchr (host, '.');
		    if (dot != NULL)
			*dot = '\0';
		}
	    } else if (flags & NI_NAMEREQD) {
		return EAI_NONAME;
	    } else if (inet_ntop (af, addr, host, hostlen) == NULL)
		return EAI_SYSTEM;
	}
    }

    if (serv != NULL) {
	if (flags & NI_NUMERICSERV) {
	    snprintf (serv, servlen, "%u", ntohs(port));
	} else {
	    const char *proto = "tcp";
	    struct servent *se;

	    if (flags & NI_DGRAM)
		proto = "udp";

	    se = getservbyport (port, proto);
	    if (se == NULL) {
		snprintf (serv, servlen, "%u", ntohs(port));
	    } else {
		strlcpy (serv, se->s_name, servlen);
	    }
	}
    }
    return 0;
}

/*
 *
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
getnameinfo(const struct sockaddr *sa, socklen_t salen,
	    char *host, size_t hostlen,
	    char *serv, size_t servlen,
	    int flags)
{
    switch (sa->sa_family) {
#ifdef HAVE_IPV6
    case AF_INET6 : {
	const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sa;

	return doit (AF_INET6, &sin6->sin6_addr, sizeof(sin6->sin6_addr),
		     sin6->sin6_port,
		     host, hostlen,
		     serv, servlen,
		     flags);
    }
#endif
    case AF_INET : {
	const struct sockaddr_in *sin4 = (const struct sockaddr_in *)sa;

	return doit (AF_INET, &sin4->sin_addr, sizeof(sin4->sin_addr),
		     sin4->sin_port,
		     host, hostlen,
		     serv, servlen,
		     flags);
    }
    default :
	return EAI_FAMILY;
    }
}
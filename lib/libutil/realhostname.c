
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/socket.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#include "libutil.h"

struct sockinet {
	u_char	si_len;
	u_char	si_family;
	u_short	si_port;
};

int
realhostname(char *host, size_t hsize, const struct in_addr *ip)
{
	char trimmed[MAXHOSTNAMELEN];
	int result;
	struct hostent *hp;

	result = HOSTNAME_INVALIDADDR;
	hp = gethostbyaddr((const char *)ip, sizeof(*ip), AF_INET);

	if (hp != NULL) {
		strlcpy(trimmed, hp->h_name, sizeof(trimmed));
		trimdomain(trimmed, strlen(trimmed));
		if (strlen(trimmed) <= hsize) {
			char lookup[MAXHOSTNAMELEN];

			strlcpy(lookup, hp->h_name, sizeof(lookup));
			hp = gethostbyname(lookup);
			if (hp == NULL)
				result = HOSTNAME_INVALIDNAME;
			else for (; ; hp->h_addr_list++) {
				if (*hp->h_addr_list == NULL) {
					result = HOSTNAME_INCORRECTNAME;
					break;
				}
				if (!memcmp(*hp->h_addr_list, ip, sizeof(*ip))) {
					strncpy(host, trimmed, hsize);
					return HOSTNAME_FOUND;
				}
			}
		}
	}

	strncpy(host, inet_ntoa(*ip), hsize);

	return result;
}

/*
 * struct sockaddr has very lax alignment requirements, since all its
 * members are char or equivalent.  This is a problem when trying to
 * dereference a struct sockaddr_in6 * that was passed in as a struct
 * sockaddr *.  Although we know (or trust) that the passed-in struct was
 * properly aligned, the compiler doesn't, and (rightly) complains.  These
 * macros perform the cast in a way that the compiler will accept.
 */
#define SOCKADDR_IN6(p) ((struct sockaddr_in6 *)(void *)(p))
#define SOCKADDR_IN(p) ((struct sockaddr_in *)(void *)(p))
#define SOCKINET(p) ((struct sockinet *)(void *)(p))

int
realhostname_sa(char *host, size_t hsize, struct sockaddr *addr, int addrlen)
{
	int result, error;
	char buf[NI_MAXHOST];
#ifdef INET6
	struct sockaddr_in lsin;
#endif

	result = HOSTNAME_INVALIDADDR;

#ifdef INET6
	/* IPv4 mapped IPv6 addr consideraton, specified in rfc2373. */
	if (addr->sa_family == AF_INET6 &&
	    addrlen == sizeof(struct sockaddr_in6) &&
	    IN6_IS_ADDR_V4MAPPED(&SOCKADDR_IN6(addr)->sin6_addr)) {
		struct sockaddr_in6 *sin6;

		sin6 = SOCKADDR_IN6(addr);

		memset(&lsin, 0, sizeof(lsin));
		lsin.sin_len = sizeof(struct sockaddr_in);
		lsin.sin_family = AF_INET;
		lsin.sin_port = sin6->sin6_port;
		memcpy(&lsin.sin_addr, &sin6->sin6_addr.s6_addr[12],
		       sizeof(struct in_addr));
		addr = (struct sockaddr *)&lsin;
		addrlen = lsin.sin_len;
	}
#endif

	error = getnameinfo(addr, addrlen, buf, sizeof(buf), NULL, 0,
			    NI_NAMEREQD);
	if (error == 0) {
		struct addrinfo hints, *res, *ores;
		struct sockaddr *sa;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = addr->sa_family;
		hints.ai_flags = AI_CANONNAME | AI_PASSIVE;
		hints.ai_socktype = SOCK_STREAM;

		error = getaddrinfo(buf, NULL, &hints, &res);
		if (error) {
			result = HOSTNAME_INVALIDNAME;
			goto numeric;
		}
		for (ores = res; ; res = res->ai_next) {
			if (res == NULL) {
				freeaddrinfo(ores);
				result = HOSTNAME_INCORRECTNAME;
				goto numeric;
			}
			sa = res->ai_addr;
			if (sa == NULL) {
				freeaddrinfo(ores);
				result = HOSTNAME_INCORRECTNAME;
				goto numeric;
			}
			if (sa->sa_len == addrlen &&
			    sa->sa_family == addr->sa_family) {
				SOCKINET(sa)->si_port = SOCKINET(addr)->si_port;
#ifdef INET6
				/*
				 * XXX: sin6_socpe_id may not been
				 * filled by DNS
				 */
				if (sa->sa_family == AF_INET6 &&
				    SOCKADDR_IN6(sa)->sin6_scope_id == 0)
					SOCKADDR_IN6(sa)->sin6_scope_id =
					    SOCKADDR_IN6(addr)->sin6_scope_id;
#endif
				if (!memcmp(sa, addr, sa->sa_len)) {
					result = HOSTNAME_FOUND;
					if (ores->ai_canonname == NULL) {
						freeaddrinfo(ores);
						goto numeric;
					}
					strlcpy(buf, ores->ai_canonname,
						sizeof(buf));
					trimdomain(buf, hsize);
					if (strlen(buf) > hsize &&
					    addr->sa_family == AF_INET) {
						freeaddrinfo(ores);
						goto numeric;
					}
					strncpy(host, buf, hsize);
					break;
				}
			}
		}
		freeaddrinfo(ores);
	} else {
    numeric:
		if (getnameinfo(addr, addrlen, buf, sizeof(buf), NULL, 0,
				NI_NUMERICHOST) == 0)
			strncpy(host, buf, hsize);
	}

	return result;
}
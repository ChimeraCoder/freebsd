
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

/* OPENBSD ORIGINAL: lib/libc/net/rresvport.c */

#include "includes.h"

#ifndef HAVE_RRESVPORT_AF

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if 0
int
rresvport(int *alport)
{
	return rresvport_af(alport, AF_INET);
}
#endif

int
rresvport_af(int *alport, sa_family_t af)
{
	struct sockaddr_storage ss;
	struct sockaddr *sa;
	u_int16_t *portp;
	int s;
	socklen_t salen;

	memset(&ss, '\0', sizeof ss);
	sa = (struct sockaddr *)&ss;

	switch (af) {
	case AF_INET:
		salen = sizeof(struct sockaddr_in);
		portp = &((struct sockaddr_in *)sa)->sin_port;
		break;
	case AF_INET6:
		salen = sizeof(struct sockaddr_in6);
		portp = &((struct sockaddr_in6 *)sa)->sin6_port;
		break;
	default:
		errno = EPFNOSUPPORT;
		return (-1);
	}
	sa->sa_family = af;
	
	s = socket(af, SOCK_STREAM, 0);
	if (s < 0)
		return (-1);

	*portp = htons(*alport);
	if (*alport < IPPORT_RESERVED - 1) {
		if (bind(s, sa, salen) >= 0)
			return (s);
		if (errno != EADDRINUSE) {
			(void)close(s);
			return (-1);
		}
	}

	*portp = 0;
	sa->sa_family = af;
	if (bindresvport_sa(s, sa) == -1) {
		(void)close(s);
		return (-1);
	}
	*alport = ntohs(*portp);
	return (s);
}

#endif /* HAVE_RRESVPORT_AF */

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

/* $Id: sample-gai.c,v 1.4 2009/09/02 23:48:02 tbox Exp $ */

#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <irs/netdb.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void
do_gai(int family, char *hostname) {
	struct addrinfo hints, *res, *res0;
	int error;
	char namebuf[1024], addrbuf[1024], servbuf[1024];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;
	error = getaddrinfo(hostname, "http", &hints, &res0);
	if (error) {
		fprintf(stderr, "getaddrinfo failed for %s,family=%d: %s\n",
			hostname, family, gai_strerror(error));
		return;
	}

	for (res = res0; res; res = res->ai_next) {
		error = getnameinfo(res->ai_addr, res->ai_addrlen,
				    addrbuf, sizeof(addrbuf),
				    NULL, 0, NI_NUMERICHOST);
		if (error == 0)
			error = getnameinfo(res->ai_addr, res->ai_addrlen,
					    namebuf, sizeof(namebuf),
					    servbuf, sizeof(servbuf), 0);
		if (error != 0) {
			fprintf(stderr, "getnameinfo failed: %s\n",
				gai_strerror(error));
		} else {
			printf("%s(%s/%s)=%s:%s\n", hostname,
			       res->ai_canonname, addrbuf, namebuf, servbuf);
		}
	}

	freeaddrinfo(res);
}

int
main(int argc, char *argv[]) {
	if (argc < 2)
		exit(1);

	do_gai(AF_INET, argv[1]);
	do_gai(AF_INET6, argv[1]);
	do_gai(AF_UNSPEC, argv[1]);

	exit(0);
}
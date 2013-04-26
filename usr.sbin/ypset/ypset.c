
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

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
#include <arpa/inet.h>

extern bool_t xdr_domainname();

static void
usage(void)
{
	fprintf(stderr, "usage: ypset [-h host] [-d domain] server\n");
	exit(1);
}

int
bind_tohost(struct sockaddr_in *sin, char *dom, char *server)
{
	struct ypbind_setdom ypsd;
	struct timeval tv;
	struct hostent *hp;
	CLIENT *client;
	int sock, port;
	int r;
	unsigned long server_addr;

	if ((port = htons(getrpcport(server, YPPROG, YPPROC_NULL, IPPROTO_UDP))) == 0)
		errx(1, "%s not running ypserv", server);

	bzero(&ypsd, sizeof ypsd);

	if ((hp = gethostbyname (server)) != NULL) {
		/* is this the most compatible way?? */
		bcopy (hp->h_addr_list[0],
		       (u_long *)&ypsd.ypsetdom_binding.ypbind_binding_addr,
		       sizeof (unsigned long));
	} else if ((long)(server_addr = inet_addr (server)) == -1) {
		errx(1, "can't find address for %s", server);
	} else
		bcopy (&server_addr,
		       (u_long *)&ypsd.ypsetdom_binding.ypbind_binding_addr,
		       sizeof (server_addr));

/*	strncpy(ypsd.ypsetdom_domain, dom, sizeof ypsd.ypsetdom_domain); */
	ypsd.ypsetdom_domain = dom;
	*(u_long *)&ypsd.ypsetdom_binding.ypbind_binding_port = port;
	ypsd.ypsetdom_vers = YPVERS;

	tv.tv_sec = 15;
	tv.tv_usec = 0;
	sock = RPC_ANYSOCK;
	client = clntudp_create(sin, YPBINDPROG, YPBINDVERS, tv, &sock);
	if (client == NULL) {
		warnx("can't yp_bind, reason: %s", yperr_string(YPERR_YPBIND));
		return (YPERR_YPBIND);
	}
	client->cl_auth = authunix_create_default();

	r = clnt_call(client, YPBINDPROC_SETDOM,
		(xdrproc_t)xdr_ypbind_setdom, &ypsd,
		(xdrproc_t)xdr_void, NULL, tv);
	if (r) {
		warnx("sorry, cannot ypset for domain %s on host - make sure ypbind was started with -ypset or -ypsetme", dom);
		clnt_destroy(client);
		return (YPERR_YPBIND);
	}
	clnt_destroy(client);
	return (0);
}

int
main(int argc, char *argv[])
{
	struct sockaddr_in sin;
	struct hostent *hent;
	char *domainname;
	int c;

	yp_get_default_domain(&domainname);

	bzero(&sin, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	while ((c = getopt(argc, argv, "h:d:")) != -1)
		switch (c) {
		case 'd':
			domainname = optarg;
			break;
		case 'h':
			if ((sin.sin_addr.s_addr = inet_addr(optarg)) == -1) {
				hent = gethostbyname(optarg);
				if (hent == NULL)
					errx(1, "host %s unknown", optarg);
				bcopy(hent->h_addr_list[0], &sin.sin_addr,
					sizeof sin.sin_addr);
			}
			break;
		default:
			usage();
		}

	if (optind + 1 != argc)
		usage();

	if (bind_tohost(&sin, domainname, argv[optind]))
		exit(1);
	exit(0);
}
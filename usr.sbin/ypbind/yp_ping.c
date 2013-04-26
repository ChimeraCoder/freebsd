
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
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#if 0
#ifndef lint
static char *sccsid = "@(#)from: clnt_udp.c 1.39 87/08/11 Copyr 1984 Sun Micro";
static char *sccsid = "@(#)from: clnt_udp.c	2.2 88/08/01 4.0 RPCSRC";
#endif
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * clnt_udp.c, Implements a UDP/IP based, client side RPC.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>
#include <rpcsvc/yp.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "yp_ping.h"

/*
 * pmap_getport.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */


static struct timeval timeout = { 1, 0 };
static struct timeval tottimeout = { 1, 0 };

/*
 * Find the mapped port for program,version.
 * Calls the pmap service remotely to do the lookup.
 * Returns 0 if no map exists.
 */
static u_short
__pmap_getport(struct sockaddr_in *address, u_long program, u_long version,
    u_int protocol)
{
	u_short port = 0;
	int sock = -1;
	register CLIENT *client;
	struct pmap parms;

	address->sin_port = htons(PMAPPORT);

	client = clntudp_bufcreate(address, PMAPPROG,
	    PMAPVERS, timeout, &sock, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client != (CLIENT *)NULL) {
		parms.pm_prog = program;
		parms.pm_vers = version;
		parms.pm_prot = protocol;
		parms.pm_port = 0;  /* not needed or used */
		if (CLNT_CALL(client, PMAPPROC_GETPORT,
			(xdrproc_t)xdr_pmap, &parms,
			(xdrproc_t)xdr_u_short, &port,
			tottimeout) != RPC_SUCCESS){
			rpc_createerr.cf_stat = RPC_PMAPFAILURE;
			clnt_geterr(client, &rpc_createerr.cf_error);
		} else if (port == 0) {
			rpc_createerr.cf_stat = RPC_PROGNOTREGISTERED;
		}
		CLNT_DESTROY(client);
	}
	if (sock != -1)
		(void)close(sock);
	address->sin_port = 0;
	return (port);
}

/*
 * Transmit to YPPROC_DOMAIN_NONACK, return immediately.
 */
static bool_t *
ypproc_domain_nonack_2_send(domainname *argp, CLIENT *clnt)
{
	static bool_t clnt_res;
	struct timeval TIMEOUT = { 0, 0 };

	memset((char *)&clnt_res, 0, sizeof (clnt_res));
	if (clnt_call(clnt, YPPROC_DOMAIN_NONACK,
		(xdrproc_t) xdr_domainname, (caddr_t) argp,
		(xdrproc_t) xdr_bool, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

/*
 * Receive response from YPPROC_DOMAIN_NONACK asynchronously.
 */
static bool_t *
ypproc_domain_nonack_2_recv(domainname *argp, CLIENT *clnt)
{
	static bool_t clnt_res;
	struct timeval TIMEOUT = { 0, 0 };

	memset((char *)&clnt_res, 0, sizeof (clnt_res));
	if (clnt_call(clnt, YPPROC_DOMAIN_NONACK,
		(xdrproc_t) NULL, (caddr_t) argp,
		(xdrproc_t) xdr_bool, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&clnt_res);
}

/*
 * "We have the machine that goes 'ping!'" -- Monty Python
 *
 * This function blasts packets at the YPPROC_DOMAIN_NONACK procedures
 * of the NIS servers listed in restricted_addrs structure.
 * Whoever replies the fastest becomes our chosen server.
 *
 * Note: THIS IS NOT A BROADCAST OPERATION! We could use clnt_broadcast()
 * for this, but that has the following problems:
 * - We only get the address of the machine that replied in the
 *   'eachresult' callback, and on multi-homed machines this can
 *   lead to confusion.
 * - clnt_broadcast() only transmits to local networks, whereas with
 *   NIS+ you can have a perfectly good server located anywhere on or
 *   off the local network.
 * - clnt_broadcast() blocks for an arbitrary amount of time which the
 *   caller can't control -- we want to avoid that.
 *
 * Also note that this has nothing to do with the NIS_PING procedure used
 * for replica updates.
 */

struct ping_req {
	struct sockaddr_in	sin;
	u_int32_t		xid;
};

int
__yp_ping(struct in_addr *restricted_addrs, int cnt, char *dom, short *port)
{
	struct timeval		tv = { 5, 0 };
	struct ping_req		**reqs;
	unsigned long		i;
	int			async;
	struct sockaddr_in	sin, *any = NULL;
	struct netbuf		addr;
	int			winner = -1;
	u_int32_t		xid_seed, xid_lookup;
	int			sock, dontblock = 1;
	CLIENT			*clnt;
	char			*foo = dom;
	int			validsrvs = 0;

	/* Set up handles. */
	reqs = calloc(1, sizeof(struct ping_req *) * cnt);
	xid_seed = time(NULL) ^ getpid();

	for (i = 0; i < cnt; i++) {
		bzero((char *)&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		bcopy((char *)&restricted_addrs[i],
			(char *)&sin.sin_addr, sizeof(struct in_addr));
		sin.sin_port = htons(__pmap_getport(&sin, YPPROG,
					YPVERS, IPPROTO_UDP));
		if (sin.sin_port == 0)
			continue;
		reqs[i] = calloc(1, sizeof(struct ping_req));
		bcopy((char *)&sin, (char *)&reqs[i]->sin, sizeof(sin));
		any = &reqs[i]->sin;
		reqs[i]->xid = xid_seed;
		xid_seed++;
		validsrvs++;
	}

	/* Make sure at least one server was assigned */
	if (!validsrvs) {
		free(reqs);
		return(-1);
	}

	/* Create RPC handle */
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	clnt = clntudp_create(any, YPPROG, YPVERS, tv, &sock);
	if (clnt == NULL) {
		close(sock);
		for (i = 0; i < cnt; i++)
			if (reqs[i] != NULL)
				free(reqs[i]);
		free(reqs);
		return(-1);
	}
	clnt->cl_auth = authunix_create_default();
	tv.tv_sec = 0;

	clnt_control(clnt, CLSET_TIMEOUT, (char *)&tv);
	async = TRUE;
	clnt_control(clnt, CLSET_ASYNC, (char *)&async);
	ioctl(sock, FIONBIO, &dontblock);

	/* Transmit */
	for (i = 0; i < cnt; i++) {
		if (reqs[i] != NULL) {
			clnt_control(clnt, CLSET_XID, (char *)&reqs[i]->xid);
			addr.len = sizeof(reqs[i]->sin);
			addr.buf = (char *) &reqs[i]->sin;
			clnt_control(clnt, CLSET_SVC_ADDR, &addr);
			ypproc_domain_nonack_2_send(&foo, clnt);
		}
	}

	/* Receive reply */
	ypproc_domain_nonack_2_recv(&foo, clnt);

	/* Got a winner -- look him up. */
	clnt_control(clnt, CLGET_XID, (char *)&xid_lookup);
	for (i = 0; i < cnt; i++) {
		if (reqs[i] != NULL && reqs[i]->xid == xid_lookup) {
			winner = i;
			*port = reqs[i]->sin.sin_port;
		}
	}

	/* Shut everything down */
	auth_destroy(clnt->cl_auth);
	clnt_destroy(clnt);
	close(sock);

	for (i = 0; i < cnt; i++)
		if (reqs[i] != NULL)
			free(reqs[i]);
	free(reqs);

	return(winner);
}
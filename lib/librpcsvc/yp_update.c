
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

#include <stdlib.h>
#include <rpc/rpc.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/ypupdate_prot.h>
#include <rpc/key_prot.h>

#ifndef WINDOW
#define WINDOW (60*60)
#endif

#ifndef TIMEOUT
#define TIMEOUT 300
#endif

int
yp_update(char *domain, char *map, unsigned int ypop, char *key, int keylen,
    char *data, int datalen)
{
	char *master;
	int rval;
	unsigned int res;
	struct ypupdate_args upargs;
	struct ypdelete_args delargs;
	CLIENT *clnt;
	char netname[MAXNETNAMELEN+1];
	des_block des_key;
	struct timeval timeout;

	/* Get the master server name for 'domain.' */
	if ((rval = yp_master(domain, map, &master)))
		return(rval);

	/* Check that ypupdated is running there. */
	if (getrpcport(master, YPU_PROG, YPU_VERS, ypop))
		return(YPERR_DOMAIN);

	/* Get a handle. */
	if ((clnt = clnt_create(master, YPU_PROG, YPU_VERS, "tcp")) == NULL)
		return(YPERR_RPC);

	/*
	 * Assemble netname of server.
	 * NOTE: It's difficult to discern from the documentation, but
	 * when you make a Secure RPC call, the netname you pass should
	 * be the netname of the guy on the other side, not your own
	 * netname. This is how the client side knows what public key
	 * to use for the initial exchange. Passing your own netname
	 * only works if the server on the other side is running under
	 * your UID.
	 */
	if (!host2netname(netname, master, domain)) {
		clnt_destroy(clnt);
		return(YPERR_BADARGS);
	}

	/* Make up a DES session key. */
	key_gendes(&des_key);

	/* Set up DES authentication. */
	if ((clnt->cl_auth = (AUTH *)authdes_create(netname, WINDOW, NULL,
			&des_key)) == NULL) {
		clnt_destroy(clnt);
		return(YPERR_RESRC);
	}

	/* Set a timeout for clnt_call(). */
	timeout.tv_usec = 0;
	timeout.tv_sec = TIMEOUT;

	/*
	 * Make the call. Note that we use clnt_call() here rather than
	 * the rpcgen-erated client stubs. We could use those stubs, but
	 * then we'd have to do some gymnastics to get at the error
	 * information to figure out what error code to send back to the
	 * caller. With clnt_call(), we get the error status returned to
	 * us right away, and we only have to exert a small amount of
	 * extra effort.
	 */
	switch (ypop) {
	case YPOP_CHANGE:
		upargs.mapname = map;
		upargs.key.yp_buf_len = keylen;
		upargs.key.yp_buf_val = key;
		upargs.datum.yp_buf_len = datalen;
		upargs.datum.yp_buf_val = data;

		if ((rval = clnt_call(clnt, YPU_CHANGE,
			(xdrproc_t)xdr_ypupdate_args, &upargs,
			(xdrproc_t)xdr_u_int, &res, timeout)) != RPC_SUCCESS) {
			if (rval == RPC_AUTHERROR)
				res = YPERR_ACCESS;
			else
				res = YPERR_RPC;
		}

		break;
	case YPOP_INSERT:
		upargs.mapname = map;
		upargs.key.yp_buf_len = keylen;
		upargs.key.yp_buf_val = key;
		upargs.datum.yp_buf_len = datalen;
		upargs.datum.yp_buf_val = data;

		if ((rval = clnt_call(clnt, YPU_INSERT,
			(xdrproc_t)xdr_ypupdate_args, &upargs,
			(xdrproc_t)xdr_u_int, &res, timeout)) != RPC_SUCCESS) {
			if (rval == RPC_AUTHERROR)
				res = YPERR_ACCESS;
			else
				res = YPERR_RPC;
		}

		break;
	case YPOP_DELETE:
		delargs.mapname = map;
		delargs.key.yp_buf_len = keylen;
		delargs.key.yp_buf_val = key;

		if ((rval = clnt_call(clnt, YPU_DELETE,
			(xdrproc_t)xdr_ypdelete_args, &delargs,
			(xdrproc_t)xdr_u_int, &res, timeout)) != RPC_SUCCESS) {
			if (rval == RPC_AUTHERROR)
				res = YPERR_ACCESS;
			else
				res = YPERR_RPC;
		}

		break;
	case YPOP_STORE:
		upargs.mapname = map;
		upargs.key.yp_buf_len = keylen;
		upargs.key.yp_buf_val = key;
		upargs.datum.yp_buf_len = datalen;
		upargs.datum.yp_buf_val = data;

		if ((rval = clnt_call(clnt, YPU_STORE,
			(xdrproc_t)xdr_ypupdate_args, &upargs,
			(xdrproc_t)xdr_u_int, &res, timeout)) != RPC_SUCCESS) {
			if (rval == RPC_AUTHERROR)
				res = YPERR_ACCESS;
			else
				res = YPERR_RPC;
		}

		break;
	default:
		res = YPERR_BADARGS;
		break;
	}

	/* All done: tear down the connection. */
	auth_destroy(clnt->cl_auth);
	clnt_destroy(clnt);
	free(master);

	return(res);
}
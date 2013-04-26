
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

#if defined(LIBC_SCCS) && !defined(lint)
static char *sccsid2 = "@(#)svc_auth_unix.c 1.28 88/02/08 Copyr 1984 Sun Micro";
static char *sccsid = "@(#)svc_auth_unix.c	2.3 88/08/01 4.0 RPCSRC";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * svc_auth_unix.c
 * Handles UNIX flavor authentication parameters on the service side of rpc.
 * There are two svc auth implementations here: AUTH_UNIX and AUTH_SHORT.
 * _svcauth_unix does full blown unix style uid,gid+gids auth,
 * _svcauth_short uses a shorthand auth to index into a cache of longhand auths.
 * Note: the shorthand has been gutted for efficiency.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/systm.h>
#include <sys/ucred.h>

#include <rpc/rpc.h>

#include <rpc/rpc_com.h>

#define MAX_MACHINE_NAME	255
#define NGRPS			16

/*
 * Unix longhand authenticator
 */
enum auth_stat
_svcauth_unix(struct svc_req *rqst, struct rpc_msg *msg)
{
	enum auth_stat stat;
	XDR xdrs;
	int32_t *buf;
	uint32_t time;
	struct xucred *xcr;
	u_int auth_len;
	size_t str_len, gid_len;
	u_int i;

	xcr = rqst->rq_clntcred;
	auth_len = (u_int)msg->rm_call.cb_cred.oa_length;
	xdrmem_create(&xdrs, msg->rm_call.cb_cred.oa_base, auth_len,
	    XDR_DECODE);
	buf = XDR_INLINE(&xdrs, auth_len);
	if (buf != NULL) {
		time = IXDR_GET_UINT32(buf);
		str_len = (size_t)IXDR_GET_UINT32(buf);
		if (str_len > MAX_MACHINE_NAME) {
			stat = AUTH_BADCRED;
			goto done;
		}
		str_len = RNDUP(str_len);
		buf += str_len / sizeof (int32_t);
		xcr->cr_uid = IXDR_GET_UINT32(buf);
		xcr->cr_groups[0] = IXDR_GET_UINT32(buf);
		gid_len = (size_t)IXDR_GET_UINT32(buf);
		if (gid_len > NGRPS) {
			stat = AUTH_BADCRED;
			goto done;
		}
		for (i = 0; i < gid_len; i++) {
			if (i + 1 < XU_NGROUPS)
				xcr->cr_groups[i + 1] = IXDR_GET_INT32(buf);
			else
				buf++;
		}
		if (gid_len + 1 > XU_NGROUPS)
			xcr->cr_ngroups = XU_NGROUPS;
		else
			xcr->cr_ngroups = gid_len + 1;

		/*
		 * five is the smallest unix credentials structure -
		 * timestamp, hostname len (0), uid, gid, and gids len (0).
		 */
		if ((5 + gid_len) * BYTES_PER_XDR_UNIT + str_len > auth_len) {
			(void) printf("bad auth_len gid %ld str %ld auth %u\n",
			    (long)gid_len, (long)str_len, auth_len);
			stat = AUTH_BADCRED;
			goto done;
		}
	} else if (! xdr_authunix_parms(&xdrs, &time, xcr)) {
		stat = AUTH_BADCRED;
		goto done;
	}

	rqst->rq_verf = _null_auth;
	stat = AUTH_OK;
done:
	XDR_DESTROY(&xdrs);

	return (stat);
}


/*
 * Shorthand unix authenticator
 * Looks up longhand in a cache.
 */
/*ARGSUSED*/
enum auth_stat 
_svcauth_short(rqst, msg)
	struct svc_req *rqst;
	struct rpc_msg *msg;
{
	return (AUTH_REJECTEDCRED);
}
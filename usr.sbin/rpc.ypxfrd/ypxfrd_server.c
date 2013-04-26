
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

#include "ypxfrd.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/fcntl.h>
#include <machine/endian.h>
#include "ypxfrd_extern.h"

int forked = 0;
int children = 0;
int fp = 0;

static bool_t
xdr_my_xfr(register XDR *xdrs, xfr *objp)
{
	unsigned char buf[XFRBLOCKSIZE];

	while (1) {
		if ((objp->xfr_u.xfrblock_buf.xfrblock_buf_len =
		    read(fp, &buf, XFRBLOCKSIZE)) != -1) {
			objp->ok = TRUE;
			objp->xfr_u.xfrblock_buf.xfrblock_buf_val = (char *)&buf;
		} else {
			objp->ok = FALSE;
			objp->xfr_u.xfrstat = XFR_READ_ERR;
			yp_error("read error: %s", strerror(errno));
		}

		/* Serialize */
		if (!xdr_xfr(xdrs, objp))
			return(FALSE);
		if (objp->ok == FALSE)
			return(TRUE);
		if (objp->xfr_u.xfrblock_buf.xfrblock_buf_len < XFRBLOCKSIZE) {
			objp->ok = FALSE;
			objp->xfr_u.xfrstat = XFR_DONE;
			if (!xdr_xfr(xdrs, objp))
				return(FALSE);
			return(TRUE);
		}
	}
}

struct xfr *
ypxfrd_getmap_1_svc(ypxfr_mapname *argp, struct svc_req *rqstp)
{
	static struct xfr  result;
	char buf[MAXPATHLEN];

	result.ok = FALSE;
	result.xfr_u.xfrstat = XFR_DENIED;

	if (yp_validdomain(argp->xfrdomain)) {
		return(&result);
	}

	if (yp_access(argp->xfrmap, (struct svc_req *)rqstp)) {
		return(&result);
	}

	snprintf (buf, sizeof(buf), "%s/%s/%s", yp_dir, argp->xfrdomain,
							argp->xfrmap);
	if (access(buf, R_OK) == -1) {
		result.xfr_u.xfrstat = XFR_ACCESS;
		return(&result);
	}

	if (argp->xfr_db_type != XFR_DB_BSD_HASH &&
	    argp->xfr_db_type != XFR_DB_ANY) {
		result.xfr_u.xfrstat = XFR_DB_TYPE_MISMATCH;
		return(&result);
	}

#if BYTE_ORDER == LITTLE_ENDIAN
	if (argp->xfr_byte_order == XFR_ENDIAN_BIG) {
#else
	if (argp->xfr_byte_order == XFR_ENDIAN_LITTLE) {
#endif
		result.xfr_u.xfrstat = XFR_DB_ENDIAN_MISMATCH;
		return(&result);
	}

#ifndef DEBUG
	if (children < MAX_CHILDREN && fork()) {
		children++;
		forked = 0;
		return (NULL);
	} else {
		forked++;
	}
#endif
	if ((fp = open(buf, O_RDONLY)) == -1) {
		result.xfr_u.xfrstat = XFR_READ_ERR;
		return(&result);
	}

	/* Start sending the file. */

	svc_sendreply(rqstp->rq_xprt, (xdrproc_t)xdr_my_xfr, &result);

	close(fp);

	return (NULL);
}
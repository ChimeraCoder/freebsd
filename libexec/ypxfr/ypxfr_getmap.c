
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

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <rpc/rpc.h>
#include <rpc/xdr.h>
#include <rpcsvc/yp.h>
#include "ypxfr_extern.h"

extern bool_t xdr_ypresp_all_seq(XDR *, unsigned long *);

int (*ypresp_allfn)();
void *ypresp_data;
extern DB *specdbp;
extern enum ypstat yp_errno;

/*
 * This is largely the same as yp_all() except we do the transfer
 * from a specific server without the aid of ypbind(8). We need to
 * be able to specify the source host explicitly since ypxfr may
 * only transfer maps from the NIS master server for any given domain.
 * However, if we use the libc version of yp_all(), we could end up
 * talking to one of the slaves instead. We do need to dig into libc
 * a little though, since it contains the magic XDR function we need.
 */
int
ypxfr_get_map(char *map, char *domain, char *host,
    int (*callback)(int, char *, int, char *, int, char*))
{
	CLIENT *clnt;
	ypreq_nokey req;
	unsigned long status;
	struct timeval timeout;

	timeout.tv_usec = 0;
	timeout.tv_sec = 10;

	/* YPPROC_ALL is a TCP service */
	if ((clnt = clnt_create(host, YPPROG, YPVERS, "tcp")) == NULL) {
		yp_error("%s", clnt_spcreateerror("failed to \
create tcp handle"));
		yp_errno = (enum ypstat)YPXFR_YPERR;
		return(1);
	}

	req.domain = domain;
	req.map = map;
	ypresp_allfn = callback;
	ypresp_data = NULL;

	(void)clnt_call(clnt, YPPROC_ALL, (xdrproc_t)xdr_ypreq_nokey, &req,
	    (xdrproc_t)xdr_ypresp_all_seq, &status, timeout);

	clnt_destroy(clnt);

	if (status == YP_NOMORE)
		return(0);

	if (status != YP_TRUE) {
		yp_errno = (enum ypstat)YPXFR_YPERR;
		return(1);
	}

	return(0);
}

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

#include <rpc/rpc.h>
#include <rpc/svc_dg.h>
#include "yp_extern.h"

#define su_data(xprt)	((struct svc_dg_data *)(xprt->xp_p2))

/*
 * We need to be able to manually set the transaction ID in the
 * UDP transport handle, but the standard library offers us no way
 * to do that. Hence we need this garbage.
 */

unsigned long
svcudp_get_xid(SVCXPRT *xprt)
{
	struct svc_dg_data *su;

	if (xprt == NULL)
		return(0);
	su = su_data(xprt);
	return(su->su_xid);
}

unsigned long
svcudp_set_xid(SVCXPRT *xprt, unsigned long xid)
{
	struct svc_dg_data *su;
	unsigned long old_xid;

	if (xprt == NULL)
		return(0);
	su = su_data(xprt);
	old_xid = su->su_xid;
	su->su_xid = xid;
	return(old_xid);
}
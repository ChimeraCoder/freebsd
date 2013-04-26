
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
#include <rpcsvc/yppasswd.h>
#include <netinet/in.h>

/*
 * XXX <rpcsvc/yppasswd.h> does a typedef that makes 'yppasswd'
 * a type of struct yppasswd. This leads to a namespace collision:
 * gcc will not let you have a type called yppasswd and a function
 * called yppasswd(). In order to get around this, we call the
 * actual function _yppasswd() and put a macro called yppasswd()
 * in yppasswd.h which calls the underlying function, thereby
 * fooling gcc.
 */

int
_yppasswd(char *oldpass, struct x_passwd *newpw)
{
	char *server;
	char *domain;
	int rval, result;
	struct yppasswd yppasswd;

	yppasswd.newpw = *newpw;
	yppasswd.oldpass = oldpass;

	if (yp_get_default_domain(&domain))
		return (-1);

	if (yp_master(domain, "passwd.byname", &server))
		return(-1);

	rval = getrpcport(server, YPPASSWDPROG,
				YPPASSWDPROC_UPDATE, IPPROTO_UDP);

	if (rval == 0 || rval >= IPPORT_RESERVED) {
		free(server);
		return(-1);
	}

	rval = callrpc(server, YPPASSWDPROG, YPPASSWDVERS, YPPASSWDPROC_UPDATE,
		       (xdrproc_t)xdr_yppasswd, (char *)&yppasswd,
		       (xdrproc_t)xdr_int, (char *)&result);

	free(server);
	if (rval || result)
		return(-1);
	else
		return(0);
}
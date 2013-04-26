
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

#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)rwall.c	1.2 91/03/11 TIRPC 1.0; from  1.3 89/03/24 SMI";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

/*
 * "High" level programmatic interface to rwall RPC service.
 */
#include <rpc/rpc.h>
#include <rpcsvc/rwall.h>

int
rwall(char *host, char *msg)
{
	return (callrpc(host, WALLPROG, WALLVERS, WALLPROC_WALL,
			(xdrproc_t)xdr_wrapstring, (char *) &msg,
			(xdrproc_t)xdr_void, (char *) NULL));
}
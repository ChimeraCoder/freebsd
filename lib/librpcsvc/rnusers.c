
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
static char sccsid[] = "@(#)rnusers.c	1.2 91/03/11 TIRPC 1.0; from 1.7 89/03/24 SMI";
#endif

/*
 * rnusers.c
 *
 * "High" level programmatic interface to rnusers RPC service.
 *
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <rpc/rpc.h>
#include <rpcsvc/rnusers.h>

int
rusers(char *host, utmpidlearr *up)
{
	return (callrpc(host, RUSERSPROG, RUSERSVERS_IDLE, RUSERSPROC_NAMES,
			(xdrproc_t)xdr_void, (char *) NULL,
			(xdrproc_t)xdr_utmpidlearr, (char *) up));
}

int
rnusers(char *host)
{
	int nusers;

	if (callrpc(host, RUSERSPROG, RUSERSVERS_ORIG, RUSERSPROC_NUM,
			(xdrproc_t)xdr_void, (char *) NULL,
			(xdrproc_t)xdr_u_long, (char *) &nusers) != 0)
		return (-1);
	else
		return (nusers);
}
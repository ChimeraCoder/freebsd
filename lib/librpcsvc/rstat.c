
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
static char sccsid[] = "@(#)rstat.c	1.2 91/03/11 TIRPC 1.0; from 1.6 89/03/24 SMI";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

/*
 * "High" level programmatic interface to rstat RPC service.
 */
#include <rpc/rpc.h>
#include <rpcsvc/rstat.h>

enum clnt_stat
rstat(char *host, struct statstime *statp)
{
	return (callrpc(host, RSTATPROG, RSTATVERS_TIME, RSTATPROC_STATS,
			(xdrproc_t)xdr_void, (char *) NULL,
			(xdrproc_t)xdr_statstime, (char *) statp));
}

int
havedisk(char *host)
{
	long have;
	
	if (callrpc(host, RSTATPROG, RSTATVERS_SWTCH, RSTATPROC_HAVEDISK,
			(xdrproc_t)xdr_void, (char *) NULL,
			(xdrproc_t)xdr_long, (char *) &have) != 0)
		return (-1);
	else
		return (have);
}
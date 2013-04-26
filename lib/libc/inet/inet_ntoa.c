
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
static const char sccsid[] = "@(#)inet_ntoa.c	8.1 (Berkeley) 6/4/93";
static const char rcsid[] = "$Id: inet_ntoa.c,v 1.1.352.1 2005/04/27 05:00:54 sra Exp $";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "port_before.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>

#include "port_after.h"

/*%
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
/*const*/ char *
inet_ntoa(struct in_addr in) {
	static char ret[18];

	strcpy(ret, "[inet_ntoa error]");
	(void) inet_ntop(AF_INET, &in, ret, sizeof ret);
	return (ret);
}

char *
inet_ntoa_r(struct in_addr in, char *buf, socklen_t size)
{

	(void) inet_ntop(AF_INET, &in, buf, size);
	return (buf);
}

/*
 * Weak aliases for applications that use certain private entry points,
 * and fail to include <arpa/inet.h>.
 */
#undef inet_ntoa
__weak_reference(__inet_ntoa, inet_ntoa);
__weak_reference(__inet_ntoa_r, inet_ntoa_r);

/*! \file */
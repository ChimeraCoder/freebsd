
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)inet_ntoa.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "stand.h"

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
char *
inet_ntoa(in)
	struct in_addr in;
{
	static const char fmt[] = "%u.%u.%u.%u";
	static char ret[sizeof "255.255.255.255"];
	unsigned char *src = (unsigned char *) &in;

	sprintf(ret, fmt, src[0], src[1], src[2], src[3]);
	return (ret);
}

/*
 * Weak aliases for applications that use certain private entry points,
 * and fail to include <arpa/inet.h>.
 */
#undef inet_ntoa
__weak_reference(__inet_ntoa, inet_ntoa);
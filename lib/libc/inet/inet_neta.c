
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
static const char rcsid[] = "$Id: inet_neta.c,v 1.2.18.1 2005/04/27 05:00:53 sra Exp $";
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "port_before.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "port_after.h"

#ifdef SPRINTF_CHAR
# define SPRINTF(x) strlen(sprintf/**/x)
#else
# define SPRINTF(x) ((size_t)sprintf x)
#endif

/*%
 * char *
 * inet_neta(src, dst, size)
 *	format an in_addr_t network number into presentation format.
 * return:
 *	pointer to dst, or NULL if an error occurred (check errno).
 * note:
 *	format of ``src'' is as for inet_network().
 * author:
 *	Paul Vixie (ISC), July 1996
 */
char *
inet_neta(src, dst, size)
	in_addr_t src;
	char *dst;
	size_t size;
{
	char *odst = dst;
	char *tp;

	while (src & 0xffffffff) {
		u_char b = (src & 0xff000000) >> 24;

		src <<= 8;
		if (b) {
			if (size < sizeof "255.")
				goto emsgsize;
			tp = dst;
			dst += SPRINTF((dst, "%u", b));
			if (src != 0L) {
				*dst++ = '.';
				*dst = '\0';
			}
			size -= (size_t)(dst - tp);
		}
	}
	if (dst == odst) {
		if (size < sizeof "0.0.0.0")
			goto emsgsize;
		strcpy(dst, "0.0.0.0");
	}
	return (odst);

 emsgsize:
	errno = EMSGSIZE;
	return (NULL);
}

/*
 * Weak aliases for applications that use certain private entry points,
 * and fail to include <arpa/inet.h>.
 */
#undef inet_neta
__weak_reference(__inet_neta, inet_neta);

/*! \file */
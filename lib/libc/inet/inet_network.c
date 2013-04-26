
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
static const char sccsid[] = "@(#)inet_network.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "port_before.h"

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "port_after.h"

/*%
 * Internet network address interpretation routine.
 * The library routines call this routine to interpret
 * network numbers.
 */
in_addr_t
inet_network(cp)
	const char *cp;
{
	in_addr_t val, base, n;
	char c;
	in_addr_t parts[4], *pp = parts;
	int i, digit;

again:
	val = 0; base = 10; digit = 0;
	if (*cp == '0')
		digit = 1, base = 8, cp++;
	if (*cp == 'x' || *cp == 'X')
		base = 16, cp++;
	while ((c = *cp) != 0) {
		if (isdigit((unsigned char)c)) {
			if (base == 8U && (c == '8' || c == '9'))
				return (INADDR_NONE);
			val = (val * base) + (c - '0');
			cp++;
			digit = 1;
			continue;
		}
		if (base == 16U && isxdigit((unsigned char)c)) {
			val = (val << 4) +
			      (c + 10 - (islower((unsigned char)c) ? 'a' : 'A'));
			cp++;
			digit = 1;
			continue;
		}
		break;
	}
	if (!digit)
		return (INADDR_NONE);
	if (pp >= parts + 4 || val > 0xffU)
		return (INADDR_NONE);
	if (*cp == '.') {
		*pp++ = val, cp++;
		goto again;
	}
	if (*cp && !isspace(*cp&0xff))
		return (INADDR_NONE);
	*pp++ = val;
	n = pp - parts;
	if (n > 4U)
		return (INADDR_NONE);
	for (val = 0, i = 0; i < n; i++) {
		val <<= 8;
		val |= parts[i] & 0xff;
	}
	return (val);
}

/*
 * Weak aliases for applications that use certain private entry points,
 * and fail to include <arpa/inet.h>.
 */
#undef inet_network
__weak_reference(__inet_network, inet_network);

/*! \file */
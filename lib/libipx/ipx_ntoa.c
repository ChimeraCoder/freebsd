
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
static char sccsid[] = "@(#)ipx_ntoa.c";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <arpa/inet.h>
#include <netipx/ipx.h>
#include <stdio.h>

static char *spectHex(char *);

char *
ipx_ntoa(addr)
	struct ipx_addr addr;
{
	static char obuf[40];
	union { union ipx_net net_e; u_long long_e; } net;
	u_short port = htons(addr.x_port);
	char *cp;
	char *cp2;
	u_char *up = addr.x_host.c_host;
	u_char *uplim = up + 6;

	net.net_e = addr.x_net;
	sprintf(obuf, "%lx", (u_long)ntohl(net.long_e));
	cp = spectHex(obuf);
	cp2 = cp + 1;
	while (*up==0 && up < uplim) up++;
	if (up == uplim) {
		if (port) {
			sprintf(cp, ".0");
			cp += 2;
		}
	} else {
		sprintf(cp, ".%x", *up++);
		while (up < uplim) {
			while (*cp) cp++;
			sprintf(cp, "%02x", *up++);
		}
		cp = spectHex(cp2);
	}
	if (port) {
		sprintf(cp, ".%x", port);
		spectHex(cp + 1);
	}
	return (obuf);
}

static char *
spectHex(p0)
	char *p0;
{
	int ok = 0;
	int nonzero = 0;
	char *p = p0;
	for (; *p; p++) switch (*p) {

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		ok = 1;
	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		nonzero = 1;
	}
	if (nonzero && !ok) { *p++ = 'H'; *p = 0; }
	return (p);
}
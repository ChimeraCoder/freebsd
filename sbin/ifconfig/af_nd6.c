
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/route.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <net/if_var.h>
#include <netinet/in_var.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <netinet6/nd6.h>

#include "ifconfig.h"

#define	MAX_SYSCTL_TRY	5
#define	ND6BITS	"\020\001PERFORMNUD\002ACCEPT_RTADV\003PREFER_SOURCE" \
		"\004IFDISABLED\005DONT_SET_IFROUTE\006AUTO_LINKLOCAL" \
		"\007NO_RADR\010NO_PREFER_IFACE\020DEFAULTIF"

static int isnd6defif(int);
void setnd6flags(const char *, int, int, const struct afswtch *);
void setnd6defif(const char *, int, int, const struct afswtch *);
void nd6_status(int);

void
setnd6flags(const char *dummyaddr __unused,
	int d, int s,
	const struct afswtch *afp)
{
	struct in6_ndireq nd;
	int error;

	memset(&nd, 0, sizeof(nd));
	strncpy(nd.ifname, ifr.ifr_name, sizeof(nd.ifname));
	error = ioctl(s, SIOCGIFINFO_IN6, &nd);
	if (error) {
		warn("ioctl(SIOCGIFINFO_IN6)");
		return;
	}
	if (d < 0)
		nd.ndi.flags &= ~(-d);
	else
		nd.ndi.flags |= d;
	error = ioctl(s, SIOCSIFINFO_IN6, (caddr_t)&nd);
	if (error)
		warn("ioctl(SIOCSIFINFO_IN6)");
}

void
setnd6defif(const char *dummyaddr __unused,
	int d, int s,
	const struct afswtch *afp)
{
	struct in6_ndifreq ndifreq;
	int ifindex;
	int error;

	memset(&ndifreq, 0, sizeof(ndifreq));
	strncpy(ndifreq.ifname, ifr.ifr_name, sizeof(ndifreq.ifname));

	if (d < 0) {
		if (isnd6defif(s)) {
			/* ifindex = 0 means to remove default if */
			ifindex = 0;
		} else
			return;
	} else if ((ifindex = if_nametoindex(ndifreq.ifname)) == 0) {
		warn("if_nametoindex(%s)", ndifreq.ifname);
		return;
	}

	ndifreq.ifindex = ifindex;
	error = ioctl(s, SIOCSDEFIFACE_IN6, (caddr_t)&ndifreq);
	if (error)
		warn("ioctl(SIOCSDEFIFACE_IN6)");
}

static int
isnd6defif(int s)
{
	struct in6_ndifreq ndifreq;
	unsigned int ifindex;
	int error;

	memset(&ndifreq, 0, sizeof(ndifreq));
	strncpy(ndifreq.ifname, ifr.ifr_name, sizeof(ndifreq.ifname));

	ifindex = if_nametoindex(ndifreq.ifname);
	error = ioctl(s, SIOCGDEFIFACE_IN6, (caddr_t)&ndifreq);
	if (error) {
		warn("ioctl(SIOCGDEFIFACE_IN6)");
		return (error);
	}
	return (ndifreq.ifindex == ifindex);
}

void
nd6_status(int s)
{
	struct in6_ndireq nd;
	int s6;
	int error;
	int isdefif;

	memset(&nd, 0, sizeof(nd));
	strncpy(nd.ifname, ifr.ifr_name, sizeof(nd.ifname));
	if ((s6 = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
		if (errno != EAFNOSUPPORT)
			warn("socket(AF_INET6, SOCK_DGRAM)");
		return;
	}
	error = ioctl(s6, SIOCGIFINFO_IN6, &nd);
	if (error) {
		if (errno != EPFNOSUPPORT)
			warn("ioctl(SIOCGIFINFO_IN6)");
		close(s6);
		return;
	}
	isdefif = isnd6defif(s6);
	close(s6);
	if (nd.ndi.flags == 0 && !isdefif)
		return;
	printb("\tnd6 options",
	    (unsigned int)(nd.ndi.flags | (isdefif << 15)), ND6BITS);
	putchar('\n');
}
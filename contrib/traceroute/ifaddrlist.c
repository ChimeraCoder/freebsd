
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
    "@(#) $Id: ifaddrlist.c,v 1.9 2000/11/23 20:01:55 leres Exp $ (LBL)";
#endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#include <sys/time.h>				/* concession to AIX */

#if __STDC__
struct mbuf;
struct rtentry;
#endif

#include <net/if.h>
#include <netinet/in.h>

#include <ctype.h>
#include <errno.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ifaddrlist.h"

/*
 * Return the interface list
 */
int
ifaddrlist(register struct ifaddrlist **ipaddrp, register char *errbuf)
{
	register int fd, nipaddr;
#ifdef HAVE_SOCKADDR_SA_LEN
	size_t n;
#endif
	register struct ifreq *ifrp, *ifend, *ifnext, *mp;
	register struct sockaddr_in *sin;
	register struct ifaddrlist *al;
	struct ifconf ifc;
	struct ifreq ibuf[(32 * 1024) / sizeof(struct ifreq)], ifr;
#define MAX_IPADDR ((int)(sizeof(ibuf) / sizeof(ibuf[0])))
	static struct ifaddrlist ifaddrlist[MAX_IPADDR];
	char device[sizeof(ifr.ifr_name) + 1];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		(void)sprintf(errbuf, "socket: %s", strerror(errno));
		return (-1);
	}
	ifc.ifc_len = sizeof(ibuf);
	ifc.ifc_buf = (caddr_t)ibuf;

	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) < 0 ||
	    ifc.ifc_len < (int)sizeof(struct ifreq)) {
		if (errno == EINVAL)
			(void)sprintf(errbuf,
			    "SIOCGIFCONF: ifreq struct too small (%zu bytes)",
			    sizeof(ibuf));
		else
			(void)sprintf(errbuf, "SIOCGIFCONF: %s",
			    strerror(errno));
		(void)close(fd);
		return (-1);
	}
	ifrp = ibuf;
	ifend = (struct ifreq *)((char *)ibuf + ifc.ifc_len);

	al = ifaddrlist;
	mp = NULL;
	nipaddr = 0;
	for (; ifrp < ifend; ifrp = ifnext) {
#ifdef HAVE_SOCKADDR_SA_LEN
		n = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
		if (n < sizeof(*ifrp))
			ifnext = ifrp + 1;
		else
			ifnext = (struct ifreq *)((char *)ifrp + n);
		if (ifrp->ifr_addr.sa_family != AF_INET)
			continue;
#else
		ifnext = ifrp + 1;
#endif
		/*
		 * Need a template to preserve address info that is
		 * used below to locate the next entry.  (Otherwise,
		 * SIOCGIFFLAGS stomps over it because the requests
		 * are returned in a union.)
		 */
		strncpy(ifr.ifr_name, ifrp->ifr_name, sizeof(ifr.ifr_name));
		if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) < 0) {
			if (errno == ENXIO)
				continue;
			(void)sprintf(errbuf, "SIOCGIFFLAGS: %.*s: %s",
			    (int)sizeof(ifr.ifr_name), ifr.ifr_name,
			    strerror(errno));
			(void)close(fd);
			return (-1);
		}

		/* Must be up */
		if ((ifr.ifr_flags & IFF_UP) == 0)
			continue;


		(void)strncpy(device, ifr.ifr_name, sizeof(ifr.ifr_name));
		device[sizeof(device) - 1] = '\0';
#ifdef sun
		/* Ignore sun virtual interfaces */
		if (strchr(device, ':') != NULL)
			continue;
#endif
		if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) < 0) {
			(void)sprintf(errbuf, "SIOCGIFADDR: %s: %s",
			    device, strerror(errno));
			(void)close(fd);
			return (-1);
		}

		if (nipaddr >= MAX_IPADDR) {
			(void)sprintf(errbuf, "Too many interfaces (%d)",
			    MAX_IPADDR);
			(void)close(fd);
			return (-1);
		}
		sin = (struct sockaddr_in *)&ifr.ifr_addr;
		al->addr = sin->sin_addr.s_addr;
		al->device = strdup(device);
		++al;
		++nipaddr;
	}
	(void)close(fd);

	*ipaddrp = ifaddrlist;
	return (nipaddr);
}
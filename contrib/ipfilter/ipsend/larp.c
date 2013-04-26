
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
/*	$FreeBSD$	*/
/*
 * larp.c (C) 1995-1998 Darren Reed
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 */
#if !defined(lint)
static const char sccsid[] = "@(#)larp.c	1.1 8/19/95 (C)1995 Darren Reed";
static const char rcsid[] = "@(#)$Id: larp.c,v 2.4 2003/12/01 02:01:16 darrenr Exp $";
#endif
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>

#include "ip_compat.h"
#include "iplang/iplang.h"

/*
 * lookup host and return
 * its IP address in address
 * (4 bytes)
 */
int	resolve(host, address)
char	*host, *address;
{
        struct	hostent	*hp;
        u_long	add;

	add = inet_addr(host);
	if (add == -1)
	    {
		if (!(hp = gethostbyname(host)))
		    {
			fprintf(stderr, "unknown host: %s\n", host);
			return -1;
		    }
		bcopy((char *)hp->h_addr, (char *)address, 4);
		return 0;
	}
	bcopy((char*)&add, address, 4);
	return 0;
}

/*
 * ARP for the MAC address corresponding
 * to the IP address.  This taken from
 * some BSD program, I cant remember which.
 */
int	arp(ip, ether)
char	*ip;
char	*ether;
{
	static	int	s = -1;
	struct	arpreq	ar;
	struct	sockaddr_in	*sin;
	char	*inet_ntoa();

#ifdef	IP_SEND
	if (arp_getipv4(ip, ether) == 0)
		return 0;
#endif
	bzero((char *)&ar, sizeof(ar));
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	bcopy(ip, (char *)&sin->sin_addr.s_addr, 4);

	if (s == -1)
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		    {
			perror("arp: socket");
			return -1;
		    }

	if (ioctl(s, SIOCGARP, (caddr_t)&ar) == -1)
	    {
		fprintf(stderr, "(%s):", inet_ntoa(sin->sin_addr));
		if (errno != ENXIO)
			perror("SIOCGARP");
		return -1;
	    }

	bcopy(ar.arp_ha.sa_data, ether, 6);
	return 0;
}
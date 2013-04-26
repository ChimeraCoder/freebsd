
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
 * Copyright (C) 2002-2004 by Darren Reed.
 * 
 * See the IPFILTER.LICENCE file for details on licencing.  
 *   
 * $Id: getifname.c,v 1.5.2.3 2006/07/14 06:12:24 darrenr Exp $ 
 */     

#include "ipf.h"

#include "kmem.h"

/*
 * Given a pointer to an interface in the kernel, return a pointer to a
 * string which is the interface name.
 */
#if 0
char *getifname(ptr)
struct ifnet *ptr;
{
#if SOLARIS || defined(__hpux)
# if SOLARIS
#  include <sys/mutex.h>
#  include <sys/condvar.h>
# endif
# ifdef __hpux
#  include "compat.h"
# endif
# include "../pfil/qif.h"
	char *ifname;
	qif_t qif;

	if ((void *)ptr == (void *)-1)
		return "!";
	if (ptr == NULL)
		return "-";

	if (kmemcpy((char *)&qif, (u_long)ptr, sizeof(qif)) == -1)
		return "X";
	ifname = strdup(qif.qf_name);
	if ((ifname != NULL) && (*ifname == '\0')) {
		free(ifname);
		return "!";
	}
	return ifname;
#else
# if defined(NetBSD) && (NetBSD >= 199905) && (NetBSD < 1991011) || \
    defined(__OpenBSD__) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))
#else
	char buf[32];
	int len;
# endif
	struct ifnet netif;

	if ((void *)ptr == (void *)-1)
		return "!";
	if (ptr == NULL)
		return "-";

	if (kmemcpy((char *)&netif, (u_long)ptr, sizeof(netif)) == -1)
		return "X";
# if defined(NetBSD) && (NetBSD >= 199905) && (NetBSD < 1991011) || \
    defined(__OpenBSD__) || defined(linux) || \
    (defined(__FreeBSD__) && (__FreeBSD_version >= 501113))
	return strdup(netif.if_xname);
# else
	if (kstrncpy(buf, (u_long)netif.if_name, sizeof(buf)) == -1)
		return "X";
	if (netif.if_unit < 10)
		len = 2;
	else if (netif.if_unit < 1000)
		len = 3;
	else if (netif.if_unit < 10000)
		len = 4;
	else
		len = 5;
	buf[sizeof(buf) - len] = '\0';
	sprintf(buf + strlen(buf), "%d", netif.if_unit % 10000);
	return strdup(buf);
# endif
#endif
}
#else
char *getifname(ptr)
struct ifnet *ptr;
{
	return "X";
}
#endif
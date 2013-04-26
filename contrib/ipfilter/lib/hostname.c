
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
 * Copyright (C) 2002-2003 by Darren Reed.
 * 
 * See the IPFILTER.LICENCE file for details on licencing.  
 *   
 * $Id: hostname.c,v 1.6.2.2 2007/01/16 02:25:22 darrenr Exp $ 
 */     

#include "ipf.h"

char *hostname(v, ip)
int v;
void *ip;
{
	static char hostbuf[MAXHOSTNAMELEN+1];
	struct hostent *hp;
	struct in_addr ipa;
	struct netent *np;

	memset(&ipa, 0, sizeof(ipa));	/* XXX gcc */

	if (v == 4) {
		ipa.s_addr = *(u_32_t *)ip;
		if (ipa.s_addr == htonl(0xfedcba98))
			return "test.host.dots";
	}

	if ((opts & OPT_NORESOLVE) == 0) {
		if (v == 4) {
			hp = gethostbyaddr(ip, 4, AF_INET);
			if (hp != NULL && hp->h_name != NULL &&
			    *hp->h_name != '\0') {
				strncpy(hostbuf, hp->h_name, sizeof(hostbuf));
				hostbuf[sizeof(hostbuf) - 1] = '\0';
				return hostbuf;
			}

			np = getnetbyaddr(ipa.s_addr, AF_INET);
			if (np != NULL && np->n_name != NULL &&
			    *np->n_name != '\0') {
				strncpy(hostbuf, np->n_name, sizeof(hostbuf));
				hostbuf[sizeof(hostbuf) - 1] = '\0';
				return hostbuf;
			}
		}
	}

	if (v == 4) {
		return inet_ntoa(ipa);
	}
#ifdef  USE_INET6
	(void) inet_ntop(AF_INET6, ip, hostbuf, sizeof(hostbuf) - 1);
	hostbuf[MAXHOSTNAMELEN] = '\0';
	return hostbuf;
#else
	return "IPv6";
#endif
}
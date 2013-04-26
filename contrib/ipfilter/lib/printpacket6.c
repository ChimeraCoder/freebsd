
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
 * Copyright (C) 2002 by Darren Reed.
 * 
 * See the IPFILTER.LICENCE file for details on licencing.  
 *   
 * $Id: printpacket6.c,v 1.3.4.1 2006/06/16 17:21:13 darrenr Exp $ 
 */     

#include "ipf.h"

/*
 * This is meant to work without the IPv6 header files being present or
 * the inet_ntop() library.
 */
void printpacket6(ip)
struct ip *ip;
{
	u_char *buf, p;
	u_short plen, *addrs;
	tcphdr_t *tcp;
	u_32_t flow;

	buf = (u_char *)ip;
	tcp = (tcphdr_t *)(buf + 40);
	p = buf[6];
	flow = ntohl(*(u_32_t *)buf);
	flow &= 0xfffff;
	plen = ntohs(*((u_short *)buf +2));
	addrs = (u_short *)buf + 4;

	printf("ip6/%d %d %#x %d", buf[0] & 0xf, plen, flow, p);
	printf(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
		ntohs(addrs[0]), ntohs(addrs[1]), ntohs(addrs[2]),
		ntohs(addrs[3]), ntohs(addrs[4]), ntohs(addrs[5]),
		ntohs(addrs[6]), ntohs(addrs[7]));
	if (plen >= 4)
		if (p == IPPROTO_TCP || p == IPPROTO_UDP)
			(void)printf(",%d", ntohs(tcp->th_sport));
	printf(" >");
	addrs += 8;
	printf(" %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
		ntohs(addrs[0]), ntohs(addrs[1]), ntohs(addrs[2]),
		ntohs(addrs[3]), ntohs(addrs[4]), ntohs(addrs[5]),
		ntohs(addrs[6]), ntohs(addrs[7]));
	if (plen >= 4)
		if (p == IPPROTO_TCP || p == IPPROTO_UDP)
			(void)printf(",%d", ntohs(tcp->th_dport));
	putchar('\n');
}
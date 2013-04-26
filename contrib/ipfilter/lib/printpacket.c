
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
 * Copyright (C) 2000-2005 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 *
 * $Id: printpacket.c,v 1.12.4.5 2007/09/09 22:15:30 darrenr Exp $
 */

#include "ipf.h"

#ifndef	IP_OFFMASK
# define	IP_OFFMASK	0x3fff
#endif


void printpacket(ip)
struct ip *ip;
{
	struct	tcphdr	*tcp;
	u_short len;
	u_short off;

	if (IP_V(ip) == 6) {
		off = 0;
		len = ntohs(((u_short *)ip)[2]) + 40;
	} else {
		off = ntohs(ip->ip_off);
		len = ntohs(ip->ip_len);
	}

	if ((opts & OPT_HEX) == OPT_HEX) {
		u_char *s;
		int i;

		for (s = (u_char *)ip, i = 0; i < len; i++) {
			printf("%02x", *s++ & 0xff);
			if (len - i > 1) {
				i++;
				printf("%02x", *s++ & 0xff);
			}
			putchar(' ');
		}
		putchar('\n');
		putchar('\n');
		return;
	}

	if (IP_V(ip) == 6) {
		printpacket6(ip);
		return;
	}

	tcp = (struct tcphdr *)((char *)ip + (IP_HL(ip) << 2));
	printf("ip #%d %d(%d) %d", ntohs(ip->ip_id), ntohs(ip->ip_len),
	       IP_HL(ip) << 2, ip->ip_p);
	if (off & IP_OFFMASK)
		printf(" @%d", (off & IP_OFFMASK) << 3);
	printf(" %s", inet_ntoa(ip->ip_src));
	if (!(off & IP_OFFMASK))
		if (ip->ip_p == IPPROTO_TCP || ip->ip_p == IPPROTO_UDP)
			printf(",%d", ntohs(tcp->th_sport));
	printf(" > ");
	printf("%s", inet_ntoa(ip->ip_dst));
	if (!(off & IP_OFFMASK)) {
		if (ip->ip_p == IPPROTO_TCP || ip->ip_p == IPPROTO_UDP)
			printf(",%d", ntohs(tcp->th_dport));
		if ((ip->ip_p == IPPROTO_TCP) && (tcp->th_flags != 0)) {
			putchar(' ');
			if (tcp->th_flags & TH_FIN)
				putchar('F');
			if (tcp->th_flags & TH_SYN)
				putchar('S');
			if (tcp->th_flags & TH_RST)
				putchar('R');
			if (tcp->th_flags & TH_PUSH)
				putchar('P');
			if (tcp->th_flags & TH_ACK)
				putchar('A');
			if (tcp->th_flags & TH_URG)
				putchar('U');
			if (tcp->th_flags & TH_ECN)
				putchar('E');
			if (tcp->th_flags & TH_CWR)
				putchar('C');
		}
	}

	putchar('\n');
}

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
/* 
 * netof - return the net address part of an ip address in a sockaddr_storage structure
 *         (zero out host part)
 */#include <stdio.h>

#include "ntp_fp.h"
#include "ntp_stdlib.h"
#include "ntp.h"

#define NUM_NETOF_BUFS	10
static struct sockaddr_storage ssbuf[NUM_NETOF_BUFS];
static int next_ssbuf = 0;

struct sockaddr_storage*
netof(
        struct sockaddr_storage* hostaddr
	)
{
	register u_int32 netnum;
        struct sockaddr_storage *netaddr;

	netaddr = &ssbuf[next_ssbuf++];
	if (next_ssbuf == NUM_NETOF_BUFS)
		next_ssbuf = 0;
        memcpy(netaddr, hostaddr, sizeof(struct sockaddr_storage));

        if(netaddr->ss_family == AF_INET) {
                netnum = ((struct sockaddr_in*)netaddr)->sin_addr.s_addr;

		/*
		 * We live in a modern CIDR world where the basement nets, which
		 * used to be class A, are now probably associated with each
		 * host address. So, for class-A nets, all bits are significant.
		 */
		if(IN_CLASSC(netnum))
		    netnum &= IN_CLASSC_NET;
		else if (IN_CLASSB(netnum))
		    netnum &= IN_CLASSB_NET;
			((struct sockaddr_in*)netaddr)->sin_addr.s_addr = netnum;
		 }
         else if(netaddr->ss_family == AF_INET6) {
		/* Here we put 0 at the local link address so we get net address */
		  memset(&((struct sockaddr_in6*)netaddr)->sin6_addr.s6_addr[8], 0, 8*sizeof(u_char));
         }

         return netaddr;
}
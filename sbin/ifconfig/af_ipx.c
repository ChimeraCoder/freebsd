
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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>

#include <net/if_var.h>
#define IPTUNNEL
#include <netipx/ipx.h>
#include <netipx/ipx_if.h>

#include "ifconfig.h"

static struct ifaliasreq ipx_addreq;
static struct ifreq ipx_ridreq;

static void
ipx_status(int s __unused, const struct ifaddrs *ifa)
{
	struct sockaddr_ipx *sipx, null_sipx;

	sipx = (struct sockaddr_ipx *)ifa->ifa_addr;
	if (sipx == NULL)
		return;

	printf("\tipx %s ", ipx_ntoa(sipx->sipx_addr));

	if (ifa->ifa_flags & IFF_POINTOPOINT) {
		sipx = (struct sockaddr_ipx *)ifa->ifa_dstaddr;
		if (sipx == NULL) {
			memset(&null_sipx, 0, sizeof(null_sipx));
			sipx = &null_sipx;
		}
		printf("--> %s ", ipx_ntoa(sipx->sipx_addr));
	}
	putchar('\n');
}

#define SIPX(x) ((struct sockaddr_ipx *) &(x))
struct sockaddr_ipx *sipxtab[] = {
	SIPX(ipx_ridreq.ifr_addr), SIPX(ipx_addreq.ifra_addr),
	SIPX(ipx_addreq.ifra_mask), SIPX(ipx_addreq.ifra_broadaddr)
};

static void
ipx_getaddr(const char *addr, int which)
{
	struct sockaddr_ipx *sipx = sipxtab[which];

	sipx->sipx_family = AF_IPX;
	sipx->sipx_len = sizeof(*sipx);
	sipx->sipx_addr = ipx_addr(addr);
	if (which == MASK)
		printf("Attempt to set IPX netmask will be ineffectual\n");
}

static void
ipx_postproc(int s, const struct afswtch *afp)
{

}

static struct afswtch af_ipx = {
	.af_name	= "ipx",
	.af_af		= AF_IPX,
	.af_status	= ipx_status,
	.af_getaddr	= ipx_getaddr,
	.af_postproc	= ipx_postproc,
	.af_difaddr	= SIOCDIFADDR,
	.af_aifaddr	= SIOCAIFADDR,
	.af_ridreq	= &ipx_ridreq,
	.af_addreq	= &ipx_addreq,
};

static __constructor void
ipx_ctor(void)
{
	af_register(&af_ipx);
}
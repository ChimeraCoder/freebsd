
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

#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/ethernet.h>

#include "ifconfig.h"

static struct ifreq link_ridreq;

static void
link_status(int s __unused, const struct ifaddrs *ifa)
{
	/* XXX no const 'cuz LLADDR is defined wrong */
	struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifa->ifa_addr;

	if (sdl != NULL && sdl->sdl_alen > 0) {
		if ((sdl->sdl_type == IFT_ETHER ||
		    sdl->sdl_type == IFT_L2VLAN ||
		    sdl->sdl_type == IFT_BRIDGE) &&
		    sdl->sdl_alen == ETHER_ADDR_LEN)
			printf("\tether %s\n",
			    ether_ntoa((struct ether_addr *)LLADDR(sdl)));
		else {
			int n = sdl->sdl_nlen > 0 ? sdl->sdl_nlen + 1 : 0;

			printf("\tlladdr %s\n", link_ntoa(sdl) + n);
		}
	}
}

static void
link_getaddr(const char *addr, int which)
{
	char *temp;
	struct sockaddr_dl sdl;
	struct sockaddr *sa = &link_ridreq.ifr_addr;

	if (which != ADDR)
		errx(1, "can't set link-level netmask or broadcast");
	if ((temp = malloc(strlen(addr) + 2)) == NULL)
		errx(1, "malloc failed");
	temp[0] = ':';
	strcpy(temp + 1, addr);
	sdl.sdl_len = sizeof(sdl);
	link_addr(temp, &sdl);
	free(temp);
	if (sdl.sdl_alen > sizeof(sa->sa_data))
		errx(1, "malformed link-level address");
	sa->sa_family = AF_LINK;
	sa->sa_len = sdl.sdl_alen;
	bcopy(LLADDR(&sdl), sa->sa_data, sdl.sdl_alen);
}

static struct afswtch af_link = {
	.af_name	= "link",
	.af_af		= AF_LINK,
	.af_status	= link_status,
	.af_getaddr	= link_getaddr,
	.af_aifaddr	= SIOCSIFLLADDR,
	.af_addreq	= &link_ridreq,
};
static struct afswtch af_ether = {
	.af_name	= "ether",
	.af_af		= AF_LINK,
	.af_status	= link_status,
	.af_getaddr	= link_getaddr,
	.af_aifaddr	= SIOCSIFLLADDR,
	.af_addreq	= &link_ridreq,
};
static struct afswtch af_lladdr = {
	.af_name	= "lladdr",
	.af_af		= AF_LINK,
	.af_status	= link_status,
	.af_getaddr	= link_getaddr,
	.af_aifaddr	= SIOCSIFLLADDR,
	.af_addreq	= &link_ridreq,
};

static __constructor void
link_ctor(void)
{
	af_register(&af_link);
	af_register(&af_ether);
	af_register(&af_lladdr);
}

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

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>

#include <netinet/in.h>
#include <net/if_var.h>		/* for struct ifaddr */
#include <netinet/in_var.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "ifconfig.h"

static struct in_aliasreq in_addreq;
static struct ifreq in_ridreq;

static void
in_status(int s __unused, const struct ifaddrs *ifa)
{
	struct sockaddr_in *sin, null_sin;
	
	memset(&null_sin, 0, sizeof(null_sin));

	sin = (struct sockaddr_in *)ifa->ifa_addr;
	if (sin == NULL)
		return;

	printf("\tinet %s ", inet_ntoa(sin->sin_addr));

	if (ifa->ifa_flags & IFF_POINTOPOINT) {
		sin = (struct sockaddr_in *)ifa->ifa_dstaddr;
		if (sin == NULL)
			sin = &null_sin;
		printf("--> %s ", inet_ntoa(sin->sin_addr));
	}

	sin = (struct sockaddr_in *)ifa->ifa_netmask;
	if (sin == NULL)
		sin = &null_sin;
	printf("netmask 0x%lx ", (unsigned long)ntohl(sin->sin_addr.s_addr));

	if (ifa->ifa_flags & IFF_BROADCAST) {
		sin = (struct sockaddr_in *)ifa->ifa_broadaddr;
		if (sin != NULL && sin->sin_addr.s_addr != 0)
			printf("broadcast %s ", inet_ntoa(sin->sin_addr));
	}

	print_vhid(ifa, " ");

	putchar('\n');
}

#define SIN(x) ((struct sockaddr_in *) &(x))
static struct sockaddr_in *sintab[] = {
	SIN(in_ridreq.ifr_addr), SIN(in_addreq.ifra_addr),
	SIN(in_addreq.ifra_mask), SIN(in_addreq.ifra_broadaddr)
};

static void
in_getaddr(const char *s, int which)
{
#define	MIN(a,b)	((a)<(b)?(a):(b))
	struct sockaddr_in *sin = sintab[which];
	struct hostent *hp;
	struct netent *np;

	sin->sin_len = sizeof(*sin);
	sin->sin_family = AF_INET;

	if (which == ADDR) {
		char *p = NULL;

		if((p = strrchr(s, '/')) != NULL) {
			const char *errstr;
			/* address is `name/masklen' */
			int masklen;
			struct sockaddr_in *min = sintab[MASK];
			*p = '\0';
			if (!isdigit(*(p + 1)))
				errstr = "invalid";
			else
				masklen = (int)strtonum(p + 1, 0, 32, &errstr);
			if (errstr != NULL) {
				*p = '/';
				errx(1, "%s: bad value (width %s)", s, errstr);
			}
			min->sin_family = AF_INET;
			min->sin_len = sizeof(*min);
			min->sin_addr.s_addr = htonl(~((1LL << (32 - masklen)) - 1) & 
				              0xffffffff);
		}
	}

	if (inet_aton(s, &sin->sin_addr))
		return;
	if ((hp = gethostbyname(s)) != 0)
		bcopy(hp->h_addr, (char *)&sin->sin_addr, 
		    MIN((size_t)hp->h_length, sizeof(sin->sin_addr)));
	else if ((np = getnetbyname(s)) != 0)
		sin->sin_addr = inet_makeaddr(np->n_net, INADDR_ANY);
	else
		errx(1, "%s: bad value", s);
#undef MIN
}

static void
in_status_tunnel(int s)
{
	char src[NI_MAXHOST];
	char dst[NI_MAXHOST];
	struct ifreq ifr;
	const struct sockaddr *sa = (const struct sockaddr *) &ifr.ifr_addr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, IFNAMSIZ);

	if (ioctl(s, SIOCGIFPSRCADDR, (caddr_t)&ifr) < 0)
		return;
	if (sa->sa_family != AF_INET)
		return;
	if (getnameinfo(sa, sa->sa_len, src, sizeof(src), 0, 0, NI_NUMERICHOST) != 0)
		src[0] = '\0';

	if (ioctl(s, SIOCGIFPDSTADDR, (caddr_t)&ifr) < 0)
		return;
	if (sa->sa_family != AF_INET)
		return;
	if (getnameinfo(sa, sa->sa_len, dst, sizeof(dst), 0, 0, NI_NUMERICHOST) != 0)
		dst[0] = '\0';

	printf("\ttunnel inet %s --> %s\n", src, dst);
}

static void
in_set_tunnel(int s, struct addrinfo *srcres, struct addrinfo *dstres)
{
	struct in_aliasreq addreq;

	memset(&addreq, 0, sizeof(addreq));
	strncpy(addreq.ifra_name, name, IFNAMSIZ);
	memcpy(&addreq.ifra_addr, srcres->ai_addr, srcres->ai_addr->sa_len);
	memcpy(&addreq.ifra_dstaddr, dstres->ai_addr, dstres->ai_addr->sa_len);

	if (ioctl(s, SIOCSIFPHYADDR, &addreq) < 0)
		warn("SIOCSIFPHYADDR");
}

static struct afswtch af_inet = {
	.af_name	= "inet",
	.af_af		= AF_INET,
	.af_status	= in_status,
	.af_getaddr	= in_getaddr,
	.af_status_tunnel = in_status_tunnel,
	.af_settunnel	= in_set_tunnel,
	.af_difaddr	= SIOCDIFADDR,
	.af_aifaddr	= SIOCAIFADDR,
	.af_ridreq	= &in_ridreq,
	.af_addreq	= &in_addreq,
};

static __constructor void
inet_ctor(void)
{

#ifndef RESCUE
	if (!feature_present("inet"))
		return;
#endif
	af_register(&af_inet);
}
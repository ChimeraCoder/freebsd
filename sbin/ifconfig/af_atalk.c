
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

#include <netatalk/at.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>

#include <arpa/inet.h>

#include "ifconfig.h"

static struct netrange at_nr;		/* AppleTalk net range */
static struct ifaliasreq at_addreq;

/* XXX  FIXME -- should use strtoul for better parsing. */
static void
setatrange(const char *range, int dummy __unused, int s, 
    const struct afswtch *afp)
{
	u_int	first = 123, last = 123;

	if (sscanf(range, "%u-%u", &first, &last) != 2
	    || first == 0 || first > 0xffff
	    || last == 0 || last > 0xffff || first > last)
		errx(1, "%s: illegal net range: %u-%u", range, first, last);
	at_nr.nr_firstnet = htons(first);
	at_nr.nr_lastnet = htons(last);
}

static void
setatphase(const char *phase, int dummy __unused, int s, 
    const struct afswtch *afp)
{
	if (!strcmp(phase, "1"))
		at_nr.nr_phase = 1;
	else if (!strcmp(phase, "2"))
		at_nr.nr_phase = 2;
	else
		errx(1, "%s: illegal phase", phase);
}

static void
at_status(int s __unused, const struct ifaddrs *ifa)
{
	struct sockaddr_at *sat, null_sat;
	struct netrange *nr;

	memset(&null_sat, 0, sizeof(null_sat));

	sat = (struct sockaddr_at *)ifa->ifa_addr;
	if (sat == NULL)
		return;
	nr = &sat->sat_range.r_netrange;
	printf("\tatalk %d.%d range %d-%d phase %d",
		ntohs(sat->sat_addr.s_net), sat->sat_addr.s_node,
		ntohs(nr->nr_firstnet), ntohs(nr->nr_lastnet), nr->nr_phase);
	if (ifa->ifa_flags & IFF_POINTOPOINT) {
		sat = (struct sockaddr_at *)ifa->ifa_dstaddr;
		if (sat == NULL)
			sat = &null_sat;
		printf("--> %d.%d",
			ntohs(sat->sat_addr.s_net), sat->sat_addr.s_node);
	}
	if (ifa->ifa_flags & IFF_BROADCAST) {
		sat = (struct sockaddr_at *)ifa->ifa_broadaddr;
		if (sat != NULL)
			printf(" broadcast %d.%d",
				ntohs(sat->sat_addr.s_net),
				sat->sat_addr.s_node);
	}

	putchar('\n');
}

static void
at_getaddr(const char *addr, int which)
{
	struct sockaddr_at *sat = (struct sockaddr_at *) &at_addreq.ifra_addr;
	u_int net, node;

	sat->sat_family = AF_APPLETALK;
	sat->sat_len = sizeof(*sat);
	if (which == MASK)
		errx(1, "AppleTalk does not use netmasks");
	if (sscanf(addr, "%u.%u", &net, &node) != 2
	    || net > 0xffff || node > 0xfe)
		errx(1, "%s: illegal address", addr);
	sat->sat_addr.s_net = htons(net);
	sat->sat_addr.s_node = node;
}

static void
at_postproc(int s, const struct afswtch *afp)
{
	struct sockaddr_at *sat = (struct sockaddr_at *) &at_addreq.ifra_addr;

	if (at_nr.nr_phase == 0)
		at_nr.nr_phase = 2;	/* Default phase 2 */
	if (at_nr.nr_firstnet == 0)
		at_nr.nr_firstnet =	/* Default range of one */
		at_nr.nr_lastnet = sat->sat_addr.s_net;
	printf("\tatalk %d.%d range %d-%d phase %d\n",
		ntohs(sat->sat_addr.s_net), sat->sat_addr.s_node,
		ntohs(at_nr.nr_firstnet), ntohs(at_nr.nr_lastnet),
		at_nr.nr_phase);
	if ((u_short) ntohs(at_nr.nr_firstnet) >
			(u_short) ntohs(sat->sat_addr.s_net)
		    || (u_short) ntohs(at_nr.nr_lastnet) <
			(u_short) ntohs(sat->sat_addr.s_net))
		errx(1, "AppleTalk address is not in range");
	sat->sat_range.r_netrange = at_nr;
}

static struct cmd atalk_cmds[] = {
	DEF_CMD_ARG("range",	setatrange),
	DEF_CMD_ARG("phase",	setatphase),
};

static struct afswtch af_atalk = {
	.af_name	= "atalk",
	.af_af		= AF_APPLETALK,
	.af_status	= at_status,
	.af_getaddr	= at_getaddr,
	.af_postproc	= at_postproc,
	.af_difaddr	= SIOCDIFADDR,
	.af_aifaddr	= SIOCAIFADDR,
	.af_ridreq	= &at_addreq,
	.af_addreq	= &at_addreq,
};

static __constructor void
atalk_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(atalk_cmds);  i++)
		cmd_register(&atalk_cmds[i]);
	af_register(&af_atalk);
#undef N
}
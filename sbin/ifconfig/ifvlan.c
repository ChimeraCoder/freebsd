
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

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <stdlib.h>
#include <unistd.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_var.h>
#include <net/if_vlan_var.h>
#include <net/route.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

#include "ifconfig.h"

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif

#define	NOTAG	((u_short) -1)

static 	struct vlanreq params = {
	.vlr_tag	= NOTAG,
};

static int
getvlan(int s, struct ifreq *ifr, struct vlanreq *vreq)
{
	bzero((char *)vreq, sizeof(*vreq));
	ifr->ifr_data = (caddr_t)vreq;

	return ioctl(s, SIOCGETVLAN, (caddr_t)ifr);
}

static void
vlan_status(int s)
{
	struct vlanreq		vreq;

	if (getvlan(s, &ifr, &vreq) != -1)
		printf("\tvlan: %d parent interface: %s\n",
		    vreq.vlr_tag, vreq.vlr_parent[0] == '\0' ?
		    "<none>" : vreq.vlr_parent);
}

static void
vlan_create(int s, struct ifreq *ifr)
{
	if (params.vlr_tag != NOTAG || params.vlr_parent[0] != '\0') {
		/*
		 * One or both parameters were specified, make sure both.
		 */
		if (params.vlr_tag == NOTAG)
			errx(1, "must specify a tag for vlan create");
		if (params.vlr_parent[0] == '\0')
			errx(1, "must specify a parent device for vlan create");
		ifr->ifr_data = (caddr_t) &params;
	}
	if (ioctl(s, SIOCIFCREATE2, ifr) < 0)
		err(1, "SIOCIFCREATE2");
}

static void
vlan_cb(int s, void *arg)
{
	if ((params.vlr_tag != NOTAG) ^ (params.vlr_parent[0] != '\0'))
		errx(1, "both vlan and vlandev must be specified");
}

static void
vlan_set(int s, struct ifreq *ifr)
{
	if (params.vlr_tag != NOTAG && params.vlr_parent[0] != '\0') {
		ifr->ifr_data = (caddr_t) &params;
		if (ioctl(s, SIOCSETVLAN, (caddr_t)ifr) == -1)
			err(1, "SIOCSETVLAN");
	}
}

static
DECL_CMD_FUNC(setvlantag, val, d)
{
	struct vlanreq vreq;
	u_long ul;
	char *endp;

	ul = strtoul(val, &endp, 0);
	if (*endp != '\0')
		errx(1, "invalid value for vlan");
	params.vlr_tag = ul;
	/* check if the value can be represented in vlr_tag */
	if (params.vlr_tag != ul)
		errx(1, "value for vlan out of range");

	if (getvlan(s, &ifr, &vreq) != -1)
		vlan_set(s, &ifr);
}

static
DECL_CMD_FUNC(setvlandev, val, d)
{
	struct vlanreq vreq;

	strlcpy(params.vlr_parent, val, sizeof(params.vlr_parent));

	if (getvlan(s, &ifr, &vreq) != -1)
		vlan_set(s, &ifr);
}

static
DECL_CMD_FUNC(unsetvlandev, val, d)
{
	struct vlanreq		vreq;

	bzero((char *)&vreq, sizeof(struct vlanreq));
	ifr.ifr_data = (caddr_t)&vreq;

	if (ioctl(s, SIOCGETVLAN, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETVLAN");

	bzero((char *)&vreq.vlr_parent, sizeof(vreq.vlr_parent));
	vreq.vlr_tag = 0;

	if (ioctl(s, SIOCSETVLAN, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETVLAN");
}

static struct cmd vlan_cmds[] = {
	DEF_CLONE_CMD_ARG("vlan",			setvlantag),
	DEF_CLONE_CMD_ARG("vlandev",			setvlandev),
	/* NB: non-clone cmds */
	DEF_CMD_ARG("vlan",				setvlantag),
	DEF_CMD_ARG("vlandev",				setvlandev),
	/* XXX For compatibility.  Should become DEF_CMD() some day. */
	DEF_CMD_OPTARG("-vlandev",			unsetvlandev),
	DEF_CMD("vlanmtu",	IFCAP_VLAN_MTU,		setifcap),
	DEF_CMD("-vlanmtu",	-IFCAP_VLAN_MTU,	setifcap),
	DEF_CMD("vlanhwtag",	IFCAP_VLAN_HWTAGGING,	setifcap),
	DEF_CMD("-vlanhwtag",	-IFCAP_VLAN_HWTAGGING,	setifcap),
	DEF_CMD("vlanhwfilter",	IFCAP_VLAN_HWFILTER,	setifcap),
	DEF_CMD("-vlanhwfilter", -IFCAP_VLAN_HWFILTER,	setifcap),
	DEF_CMD("-vlanhwtso",	-IFCAP_VLAN_HWTSO,	setifcap),
	DEF_CMD("vlanhwtso",	IFCAP_VLAN_HWTSO,	setifcap),
	DEF_CMD("vlanhwcsum",	IFCAP_VLAN_HWCSUM,	setifcap),
	DEF_CMD("-vlanhwcsum",	-IFCAP_VLAN_HWCSUM,	setifcap),
};
static struct afswtch af_vlan = {
	.af_name	= "af_vlan",
	.af_af		= AF_UNSPEC,
	.af_other_status = vlan_status,
};

static __constructor void
vlan_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	size_t i;

	for (i = 0; i < N(vlan_cmds);  i++)
		cmd_register(&vlan_cmds[i]);
	af_register(&af_vlan);
	callback_register(vlan_cb, NULL);
	clone_setdefcallback("vlan", vlan_create);
#undef N
}

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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ipx.h"

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/kernel.h>
#include <sys/queue.h>
#include <sys/sysctl.h>

#include <net/radix.h>

#include <netipx/ipx.h>
#include <netipx/ipx_var.h>
#include <netipx/spx.h>

static	struct pr_usrreqs nousrreqs;

/*
 * IPX protocol family: IPX, ERR, PXP, SPX, ROUTE.
 */

static	struct domain ipxdomain;

static struct protosw ipxsw[] = {
{
	.pr_domain =		&ipxdomain,
	.pr_init =		ipx_init,
	.pr_usrreqs =		&nousrreqs
},
{
	.pr_type =		SOCK_DGRAM,
	.pr_domain =		&ipxdomain,
	.pr_flags =		PR_ATOMIC|PR_ADDR,
	.pr_ctlinput =		ipx_ctlinput,
	.pr_ctloutput =		ipx_ctloutput,
	.pr_usrreqs =		&ipx_usrreqs
},
{
	.pr_type =		SOCK_STREAM,
	.pr_domain =		&ipxdomain,
	.pr_protocol =		IPXPROTO_SPX,
	.pr_flags =		PR_CONNREQUIRED|PR_WANTRCVD,
	.pr_ctlinput =		spx_ctlinput,
	.pr_ctloutput =		spx_ctloutput,
	.pr_init =		spx_init,
	.pr_fasttimo =		spx_fasttimo,
	.pr_slowtimo =		spx_slowtimo,
	.pr_usrreqs =		&spx_usrreqs
},
{
	.pr_type =		SOCK_SEQPACKET,
	.pr_domain =		&ipxdomain,
	.pr_protocol =		IPXPROTO_SPX,
	.pr_flags =		PR_CONNREQUIRED|PR_WANTRCVD|PR_ATOMIC,
	.pr_ctlinput =		spx_ctlinput,
	.pr_ctloutput =		spx_ctloutput,
	.pr_usrreqs =		&spx_usrreq_sps
},
{
	.pr_type =		SOCK_RAW,
	.pr_domain =		&ipxdomain,
	.pr_protocol =		IPXPROTO_RAW,
	.pr_flags =		PR_ATOMIC|PR_ADDR,
	.pr_ctloutput =		ipx_ctloutput,
	.pr_usrreqs =		&ripx_usrreqs
},
};

extern int ipx_inithead(void **, int);

static struct	domain ipxdomain = {
	.dom_family =		AF_IPX,
	.dom_name =		"network systems",
	.dom_protosw =		ipxsw,
	.dom_protoswNPROTOSW =	&ipxsw[sizeof(ipxsw)/sizeof(ipxsw[0])],
	.dom_rtattach =		ipx_inithead,
	.dom_rtoffset =		16,
	.dom_maxrtkey =		sizeof(struct sockaddr_ipx)
};


/* shim to adapt arguments */
int
ipx_inithead(void **head, int offset)
{
	return rn_inithead(head, offset);
}

DOMAIN_SET(ipx);
SYSCTL_NODE(_net,	PF_IPX,		ipx,	CTLFLAG_RW, 0,
	"IPX/SPX");

SYSCTL_NODE(_net_ipx,	IPXPROTO_RAW,	ipx,	CTLFLAG_RW, 0, "IPX");
static SYSCTL_NODE(_net_ipx, IPXPROTO_SPX, spx,	CTLFLAG_RW, 0, "SPX");
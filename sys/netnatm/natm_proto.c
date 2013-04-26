
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
 * protocol layer for access to native mode ATM
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/protosw.h>
#include <sys/domain.h>

#include <net/if.h>
#include <net/netisr.h>

#include <netinet/in.h>

#include <netnatm/natm.h>

static	void natm_init(void);

static struct domain natmdomain;

static struct protosw natmsw[] = {
{
	.pr_type =		SOCK_STREAM,
	.pr_domain =		&natmdomain,
	.pr_protocol =		PROTO_NATMAAL5,
	.pr_flags =		PR_CONNREQUIRED,
	.pr_usrreqs =		&natm_usrreqs
},
{
	.pr_type =		SOCK_DGRAM,
	.pr_domain =		&natmdomain,
	.pr_protocol =		PROTO_NATMAAL5,
	.pr_flags =		PR_CONNREQUIRED|PR_ATOMIC,
	.pr_usrreqs =		&natm_usrreqs
},
{
	.pr_type =		SOCK_STREAM,
	.pr_domain =		&natmdomain,
	.pr_protocol =		PROTO_NATMAAL0,
	.pr_flags =		PR_CONNREQUIRED,
	.pr_usrreqs =		&natm_usrreqs
},
};

static struct domain natmdomain = {
	.dom_family =		AF_NATM,
	.dom_name =		"natm",
	.dom_init =		natm_init,
	.dom_protosw =		natmsw,
	.dom_protoswNPROTOSW =	&natmsw[sizeof(natmsw)/sizeof(natmsw[0])],
};

static struct netisr_handler natm_nh = {
	.nh_name = "natm",
	.nh_handler = natmintr,
	.nh_proto = NETISR_NATM,
	.nh_qlimit = 1000,
	.nh_policy = NETISR_POLICY_SOURCE,
};

#ifdef NATM_STAT
u_int natm_sodropcnt;		/* # mbufs dropped due to full sb */
u_int natm_sodropbytes;		/* # of bytes dropped */
u_int natm_sookcnt;		/* # mbufs ok */
u_int natm_sookbytes;		/* # of bytes ok */
#endif

static void
natm_init(void)
{
	LIST_INIT(&natm_pcbs);
	NATM_LOCK_INIT();
	netisr_register(&natm_nh);
}

DOMAIN_SET(natm);
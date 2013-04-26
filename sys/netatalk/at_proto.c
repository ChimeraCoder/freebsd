
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
#include <sys/protosw.h>
#include <sys/domain.h>
#include <sys/socket.h>

#include <sys/kernel.h>

#include <net/route.h>

#include <netatalk/at.h>
#include <netatalk/ddp_var.h>
#include <netatalk/at_extern.h>

static struct domain	atalkdomain;

static struct protosw	atalksw[] = {
	{
		/* Identifiers */
		.pr_type =		SOCK_DGRAM,
		.pr_domain =		&atalkdomain,
		.pr_protocol =		ATPROTO_DDP,
		.pr_flags =		PR_ATOMIC|PR_ADDR,
		.pr_output =		ddp_output,
		.pr_init =		ddp_init,
		.pr_usrreqs =		&ddp_usrreqs,
	},
};

static struct domain	atalkdomain = {
	.dom_family =		AF_APPLETALK,
	.dom_name =		"appletalk",
	.dom_protosw =		atalksw,
	.dom_protoswNPROTOSW =	&atalksw[sizeof(atalksw)/sizeof(atalksw[0])],
	.dom_rtattach =		at_inithead,
	.dom_rtoffset =		offsetof(struct sockaddr_at, sat_addr) << 3,
	.dom_maxrtkey =		sizeof(struct sockaddr_at),
};

DOMAIN_SET(atalk);
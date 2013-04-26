
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

#ifndef lint
static const char sccsid[] = "@(#)look_up.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <protocols/talkd.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "talk_ctl.h"
#include "talk.h"

/*
 * See if the local daemon has an invitation for us.
 */
int
check_local(void)
{
	CTL_RESPONSE response;
	CTL_RESPONSE *rp = &response;
	struct sockaddr addr;

	/* the rest of msg was set up in get_names */
#ifdef MSG_EOR
	/* copy new style sockaddr to old, swap family (short in old) */
	msg.ctl_addr = *(struct osockaddr *)&ctl_addr;
	msg.ctl_addr.sa_family = htons(ctl_addr.sin_family);
#else
	msg.ctl_addr = *(struct sockaddr *)&ctl_addr;
#endif
	/* must be initiating a talk */
	if (!look_for_invite(rp))
		return (0);
	/*
	 * There was an invitation waiting for us,
	 * so connect with the other (hopefully waiting) party
	 */
	current_state = "Waiting to connect with caller";
	do {
		if (rp->addr.sa_family != AF_INET)
			p_error("Response uses invalid network address");
		(void)memcpy(&addr, &rp->addr.sa_family, sizeof(addr));
		addr.sa_family = rp->addr.sa_family;
		addr.sa_len = sizeof(addr);
		errno = 0;
		if (connect(sockt, &addr, sizeof(addr)) != -1)
			return (1);
	} while (errno == EINTR);
	if (errno == ECONNREFUSED) {
		/*
		 * The caller gave up, but his invitation somehow
		 * was not cleared. Clear it and initiate an
		 * invitation. (We know there are no newer invitations,
		 * the talkd works LIFO.)
		 */
		ctl_transact(his_machine_addr, msg, DELETE, rp);
		close(sockt);
		open_sockt();
		return (0);
	}
	p_error("Unable to connect with initiator");
	/*NOTREACHED*/
	return (0);
}

/*
 * Look for an invitation on 'machine'
 */
int
look_for_invite(CTL_RESPONSE *rp)
{
	current_state = "Checking for invitation on caller's machine";
	ctl_transact(his_machine_addr, msg, LOOK_UP, rp);
	/* the switch is for later options, such as multiple invitations */
	switch (rp->answer) {

	case SUCCESS:
		msg.id_num = htonl(rp->id_num);
		return (1);

	default:
		/* there wasn't an invitation waiting for us */
		return (0);
	}
}
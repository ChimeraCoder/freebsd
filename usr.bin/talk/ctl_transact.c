
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
static const char sccsid[] = "@(#)ctl_transact.c	8.1 (Berkeley) 6/6/93";
#endif

#include <arpa/inet.h>

#include <errno.h>

#include "talk.h"
#include "talk_ctl.h"

#define CTL_WAIT 2	/* time to wait for a response, in seconds */

/*
 * SOCKDGRAM is unreliable, so we must repeat messages if we have
 * not received an acknowledgement within a reasonable amount
 * of time
 */
void
ctl_transact(struct in_addr target, CTL_MSG lmsg, int type, CTL_RESPONSE *rp)
{
	fd_set read_mask, ctl_mask;
	int nready = 0, cc;
	struct timeval wait;

	lmsg.type = type;
	daemon_addr.sin_addr = target;
	daemon_addr.sin_port = daemon_port;
	FD_ZERO(&ctl_mask);
	FD_SET(ctl_sockt, &ctl_mask);

	/*
	 * Keep sending the message until a response of
	 * the proper type is obtained.
	 */
	do {
		wait.tv_sec = CTL_WAIT;
		wait.tv_usec = 0;
		/* resend message until a response is obtained */
		do {
			cc = sendto(ctl_sockt, (char *)&lmsg, sizeof (lmsg), 0,
			    (struct sockaddr *)&daemon_addr,
			    sizeof (daemon_addr));
			if (cc != sizeof (lmsg)) {
				if (errno == EINTR)
					continue;
				p_error("Error on write to talk daemon");
			}
			read_mask = ctl_mask;
			nready = select(32, &read_mask, 0, 0, &wait);
			if (nready < 0) {
				if (errno == EINTR)
					continue;
				p_error("Error waiting for daemon response");
			}
		} while (nready == 0);
		/*
		 * Keep reading while there are queued messages
		 * (this is not necessary, it just saves extra
		 * request/acknowledgements being sent)
		 */
		do {
			cc = recv(ctl_sockt, (char *)rp, sizeof (*rp), 0);
			if (cc < 0) {
				if (errno == EINTR)
					continue;
				p_error("Error on read from talk daemon");
			}
			read_mask = ctl_mask;
			/* an immediate poll */
			timerclear(&wait);
			nready = select(32, &read_mask, 0, 0, &wait);
		} while (nready > 0 && (rp->vers != TALK_VERSION ||
		    rp->type != type));
	} while (rp->vers != TALK_VERSION || rp->type != type);
	rp->id_num = ntohl(rp->id_num);
	rp->addr.sa_family = ntohs(rp->addr.sa_family);
}
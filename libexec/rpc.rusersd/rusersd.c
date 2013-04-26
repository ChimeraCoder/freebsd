
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

#include <stdlib.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <signal.h>
#include <syslog.h>
#include <rpcsvc/rnusers.h>

#include "extern.h"

int from_inetd = 1;

static void
cleanup(int sig __unused)
{
	(void) rpcb_unset(RUSERSPROG, RUSERSVERS_IDLE, NULL);
	(void) rpcb_unset(RUSERSPROG, RUSERSVERS_ORIG, NULL);
	exit(0);
}

int
main(int argc __unused, char *argv[] __unused)
{
	SVCXPRT *transp = NULL; /* Keep compiler happy. */
	int ok;
	struct sockaddr_storage from;
	socklen_t fromlen;

	/*
	 * See if inetd started us
	 */
	fromlen = sizeof(from);
	if (getsockname(0, (struct sockaddr *)&from, &fromlen) < 0) {
		from_inetd = 0;
	}

	if (!from_inetd) {
		daemon(0, 0);

		(void) rpcb_unset(RUSERSPROG, RUSERSVERS_IDLE, NULL);
		(void) rpcb_unset(RUSERSPROG, RUSERSVERS_ORIG, NULL);

		(void) signal(SIGINT, cleanup);
		(void) signal(SIGTERM, cleanup);
		(void) signal(SIGHUP, cleanup);
	}

	openlog("rpc.rusersd", LOG_CONS|LOG_PID, LOG_DAEMON);

	if (from_inetd) {
		transp = svc_tli_create(0, NULL, NULL, 0, 0);
		if (transp == NULL) {
			syslog(LOG_ERR, "cannot create udp service.");
			exit(1);
		}
		ok = svc_reg(transp, RUSERSPROG, RUSERSVERS_IDLE,
			     rusers_service, NULL);
	} else
		ok = svc_create(rusers_service,
				RUSERSPROG, RUSERSVERS_IDLE, "udp");
	if (!ok) {
		syslog(LOG_ERR, "unable to register (RUSERSPROG, RUSERSVERS_IDLE, %s)", (!from_inetd)?"udp":"(inetd)");
		exit(1);
	}
	if (from_inetd)
		ok = svc_reg(transp, RUSERSPROG, RUSERSVERS_ORIG,
			     rusers_service, NULL);
	else
		ok = svc_create(rusers_service,
				RUSERSPROG, RUSERSVERS_ORIG, "udp");
	if (!ok) {
		syslog(LOG_ERR, "unable to register (RUSERSPROG, RUSERSVERS_ORIG, %s)", (!from_inetd)?"udp":"(inetd)");
		exit(1);
	}

	svc_run();
	syslog(LOG_ERR, "svc_run returned");
	exit(1);
}
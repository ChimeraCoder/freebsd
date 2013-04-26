
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

#include <rpc/rpc.h>
#include <rpcsvc/spray.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>

static void spray_service(struct svc_req *, SVCXPRT *);

static int from_inetd = 1;

#define	timersub(tvp, uvp, vvp)						\
	do {								\
		(vvp)->tv_sec = (tvp)->tv_sec - (uvp)->tv_sec;		\
		(vvp)->tv_usec = (tvp)->tv_usec - (uvp)->tv_usec;	\
		if ((vvp)->tv_usec < 0) {				\
			(vvp)->tv_sec--;				\
			(vvp)->tv_usec += 1000000;			\
		}							\
	} while (0)

#define TIMEOUT 120

void
cleanup(int sig __unused)
{
	(void) rpcb_unset(SPRAYPROG, SPRAYVERS, NULL);
	exit(0);
}

void
die(int sig __unused)
{
	exit(0);
}

int
main(int argc, char *argv[])
{
	SVCXPRT *transp;
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

		(void) rpcb_unset(SPRAYPROG, SPRAYVERS, NULL);

		(void) signal(SIGINT, cleanup);
		(void) signal(SIGTERM, cleanup);
		(void) signal(SIGHUP, cleanup);
	} else {
		(void) signal(SIGALRM, die);
		alarm(TIMEOUT);
	}

	openlog("rpc.sprayd", LOG_CONS|LOG_PID, LOG_DAEMON);

	if (from_inetd) {
		transp = svc_tli_create(0, NULL, NULL, 0, 0);
		if (transp == NULL) {
			syslog(LOG_ERR, "cannot create udp service.");
			exit(1);
		}
		ok = svc_reg(transp, SPRAYPROG, SPRAYVERS,
			     spray_service, NULL);
	} else
		ok = svc_create(spray_service,
				SPRAYPROG, SPRAYVERS, "udp");
	if (!ok) {
		syslog(LOG_ERR,
		    "unable to register (SPRAYPROG, SPRAYVERS, %s)",
		    (!from_inetd)?"udp":"(inetd)");
		return 1;
	}

	svc_run();
	syslog(LOG_ERR, "svc_run returned");
	return 1;
}


static void
spray_service(struct svc_req *rqstp, SVCXPRT *transp)
{
	static spraycumul scum;
	static struct timeval clear, get;

	switch (rqstp->rq_proc) {
	case SPRAYPROC_CLEAR:
		scum.counter = 0;
		(void) gettimeofday(&clear, 0);
		/*FALLTHROUGH*/

	case NULLPROC:
		(void)svc_sendreply(transp, (xdrproc_t)xdr_void, NULL);
		return;

	case SPRAYPROC_SPRAY:
		scum.counter++;
		return;

	case SPRAYPROC_GET:
		(void) gettimeofday(&get, 0);
		timersub(&get, &clear, &get);
		scum.clock.sec = get.tv_sec;
		scum.clock.usec = get.tv_usec;
		break;

	default:
		svcerr_noproc(transp);
		return;
	}

	if (!svc_sendreply(transp, (xdrproc_t)xdr_spraycumul, &scum)) {
		svcerr_systemerr(transp);
		syslog(LOG_ERR, "bad svc_sendreply");
	}
}
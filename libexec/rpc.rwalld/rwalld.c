
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

#include <err.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <rpc/rpc.h>
#include <rpcsvc/rwall.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef OSF
#define WALL_CMD "/usr/sbin/wall"
#else
#define WALL_CMD "/usr/bin/wall -n"
#endif

void wallprog_1(struct svc_req *rqstp, SVCXPRT *transp);
void possess(void);
void killkids(int sig);
static void usage(void);

int nodaemon = 0;
int from_inetd = 1;

int
main(int argc, char *argv[])
{
	SVCXPRT *transp;
	socklen_t salen;
	int ok;
	struct sockaddr_storage sa;

	if (argc == 2 && !strcmp(argv[1], "-n"))
		nodaemon = 1;
	if (argc != 1 && !nodaemon)
		usage();

	if (geteuid() == 0) {
		struct passwd *pep = getpwnam("nobody");
		if (pep)
			setuid(pep->pw_uid);
		else
			setuid(getuid());
	}

        /*
         * See if inetd started us
         */
	salen = sizeof(sa);
        if (getsockname(0, (struct sockaddr *)&sa, &salen) < 0) {
                from_inetd = 0;
        }

        if (!from_inetd) {
                if (!nodaemon)
                        possess();

		(void)rpcb_unset(WALLPROG, WALLVERS, NULL);
        }

	(void)signal(SIGCHLD, killkids);

	openlog("rpc.rwalld", LOG_CONS|LOG_PID, LOG_DAEMON);

	/* create and register the service */
	if (from_inetd) {
		transp = svc_tli_create(0, NULL, NULL, 0, 0);
		if (transp == NULL) {
			syslog(LOG_ERR, "couldn't create udp service.");
			exit(1);
		}
		ok = svc_reg(transp, WALLPROG, WALLVERS,
			     wallprog_1, NULL);
	} else
		ok = svc_create(wallprog_1,
				WALLPROG, WALLVERS, "udp");
	if (!ok) {
		syslog(LOG_ERR, "unable to register (WALLPROG, WALLVERS, %s)", (!from_inetd)?"udp":"(inetd)");
		exit(1);
	}
	svc_run();
	syslog(LOG_ERR, "svc_run returned");
	exit(1);
}

static void
usage(void)
{
	fprintf(stderr, "usage: rpc.rwalld [-n]\n");
	exit(1);
}

void
possess(void)
{
	daemon(0, 0);
}

void
killkids(int sig __unused)
{
	while(wait4(-1, NULL, WNOHANG, NULL) > 0)
		;
}

void *
wallproc_wall_1_svc(wrapstring *s, struct svc_req *rqstp __unused)
{
	static void		*dummy = NULL;

	/* fork, popen wall with special option, and send the message */
	if (fork() == 0) {
		FILE *pfp;

		pfp = popen(WALL_CMD, "w");
		if (pfp != NULL) {
			fprintf(pfp, "\007\007%s", *s);
			pclose(pfp);
			exit(0);
		}
	}
	return(&dummy);
}

void
wallprog_1(struct svc_req *rqstp, SVCXPRT *transp)
{
	union {
		char *wallproc_wall_1_arg;
	} argument;
	char *result;
	bool_t (*xdr_argument)(), (*xdr_result)();
	char *(*local)();

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void)svc_sendreply(transp, (xdrproc_t)xdr_void, NULL);
		goto leave;

	case WALLPROC_WALL:
		xdr_argument = xdr_wrapstring;
		xdr_result = xdr_void;
		local = (char *(*)()) wallproc_wall_1_svc;
		break;

	default:
		svcerr_noproc(transp);
		goto leave;
	}
	bzero(&argument, sizeof(argument));
	if (!svc_getargs(transp, (xdrproc_t)xdr_argument, &argument)) {
		svcerr_decode(transp);
		goto leave;
	}
	result = (*local)(&argument, rqstp);
	if (result != NULL &&
	    !svc_sendreply(transp, (xdrproc_t)xdr_result, result)) {
		svcerr_systemerr(transp);
	}
	if (!svc_freeargs(transp, (xdrproc_t)xdr_argument, &argument)) {
		syslog(LOG_ERR, "unable to free arguments");
		exit(1);
	}
leave:
        if (from_inetd)
                exit(0);
}

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

#include "ypupdate_prot.h"
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <rpc/pmap_clnt.h> /* for pmap_unset */
#include <rpc/rpc_com.h>
#include <string.h> /* strcmp */
#include <signal.h>
#ifdef __cplusplus
#include <sysent.h> /* getdtablesize, open */
#endif /* __cplusplus */
#include <memory.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>
#include <sys/wait.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include "ypupdated_extern.h"
#include "yp_extern.h"

#ifndef SIG_PF
#define	SIG_PF void(*)(int)
#endif

#ifdef DEBUG
#define	RPC_SVC_FG
#endif

#define	_RPCSVC_CLOSEDOWN 120
int _rpcpmstart;		/* Started by a port monitor ? */
static int _rpcfdtype;
		 /* Whether Stream or Datagram ? */
	/* States a server can be in wrt request */

#define	_IDLE 0
#define	_SERVED 1
#define	_SERVING 2

extern int _rpcsvcstate;	 /* Set when a request is serviced */

char *progname = "rpc.ypupdated";
char *yp_dir = "/var/yp/";

static void
_msgout(char* msg)
{
#ifdef RPC_SVC_FG
	if (_rpcpmstart)
		syslog(LOG_ERR, "%s", msg);
	else
		warnx("%s", msg);
#else
	syslog(LOG_ERR, "%s", msg);
#endif
}

static void
closedown(int sig)
{
	if (_rpcsvcstate == _IDLE) {
		extern fd_set svc_fdset;
		static int size;
		int i, openfd;

		if (_rpcfdtype == SOCK_DGRAM)
			exit(0);
		if (size == 0) {
			size = getdtablesize();
		}
		for (i = 0, openfd = 0; i < size && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1)
			exit(0);
	}
	if (_rpcsvcstate == _SERVED)
		_rpcsvcstate = _IDLE;

	(void) signal(SIGALRM, (SIG_PF) closedown);
	(void) alarm(_RPCSVC_CLOSEDOWN/2);
}

static void
ypupdated_svc_run(void)
{
#ifdef FD_SETSIZE
	fd_set readfds;
#else
	int readfds;
#endif /* def FD_SETSIZE */
	extern int forked;
	int pid;
	int fd_setsize = _rpc_dtablesize();

	/* Establish the identity of the parent ypupdated process. */
	pid = getpid();

	for (;;) {
#ifdef FD_SETSIZE
		readfds = svc_fdset;
#else
		readfds = svc_fds;
#endif /* def FD_SETSIZE */
		switch (select(fd_setsize, &readfds, NULL, NULL,
			       (struct timeval *)0)) {
		case -1:
			if (errno == EINTR) {
				continue;
			}
			warn("svc_run: - select failed");
			return;
		case 0:
			continue;
		default:
			svc_getreqset(&readfds);
			if (forked && pid != getpid())
				exit(0);
		}
	}
}

static void
reaper(int sig)
{
	int status;

	if (sig == SIGHUP) {
#ifdef foo
		load_securenets();
#endif
		return;
	}

	if (sig == SIGCHLD) {
		while (wait3(&status, WNOHANG, NULL) > 0)
			children--;
	} else {
		(void) pmap_unset(YPU_PROG, YPU_VERS);
		exit(0);
	}
}

void
usage(void)
{
	fprintf(stderr, "rpc.ypupdatedd [-p path]\n");
	exit(0);
}

int
main(int argc, char *argv[])
{
	register SVCXPRT *transp = NULL;
	int sock;
	int proto = 0;
	struct sockaddr_in saddr;
	int asize = sizeof (saddr);
	int ch;

	while ((ch = getopt(argc, argv, "p:h")) != -1) {
		switch (ch) {
		case 'p':
			yp_dir = optarg;
			break;
		default:
			usage();
			break;
		}
	}
#ifdef foo
	load_securenets();
#endif

	if (svc_auth_reg(AUTH_DES, _svcauth_des) == -1) {
		yp_error("failed to register AUTH_DES flavor");
		exit(1);
	}

	if (getsockname(0, (struct sockaddr *)&saddr, &asize) == 0) {
		int ssize = sizeof (int);

		if (saddr.sin_family != AF_INET)
			exit(1);
		if (getsockopt(0, SOL_SOCKET, SO_TYPE,
				(char *)&_rpcfdtype, &ssize) == -1)
			exit(1);
		sock = 0;
		_rpcpmstart = 1;
		proto = 0;
		openlog("rpc.ypupdatedd", LOG_PID, LOG_DAEMON);
	} else {
#ifndef RPC_SVC_FG
		if (daemon(0,0)) {
			err(1, "cannot fork");
		}
		openlog("rpc.ypupdated", LOG_PID, LOG_DAEMON);
#endif
		sock = RPC_ANYSOCK;
		(void) pmap_unset(YPU_PROG, YPU_VERS);
	}

	if ((_rpcfdtype == 0) || (_rpcfdtype == SOCK_DGRAM)) {
		transp = svcudp_create(sock);
		if (transp == NULL) {
			_msgout("cannot create udp service.");
			exit(1);
		}
		if (!_rpcpmstart)
			proto = IPPROTO_UDP;
		if (!svc_register(transp, YPU_PROG, YPU_VERS, ypu_prog_1, proto)) {
			_msgout("unable to register (YPU_PROG, YPU_VERS, udp).");
			exit(1);
		}
	}

	if ((_rpcfdtype == 0) || (_rpcfdtype == SOCK_STREAM)) {
		transp = svctcp_create(sock, 0, 0);
		if (transp == NULL) {
			_msgout("cannot create tcp service.");
			exit(1);
		}
		if (!_rpcpmstart)
			proto = IPPROTO_TCP;
		if (!svc_register(transp, YPU_PROG, YPU_VERS, ypu_prog_1, proto)) {
			_msgout("unable to register (YPU_PROG, YPU_VERS, tcp).");
			exit(1);
		}
	}

	if (transp == (SVCXPRT *)NULL) {
		_msgout("could not create a handle");
		exit(1);
	}
	if (_rpcpmstart) {
		(void) signal(SIGALRM, (SIG_PF) closedown);
		(void) alarm(_RPCSVC_CLOSEDOWN/2);
	}

	(void) signal(SIGPIPE, SIG_IGN);
	(void) signal(SIGCHLD, (SIG_PF) reaper);
	(void) signal(SIGTERM, (SIG_PF) reaper);
	(void) signal(SIGINT, (SIG_PF) reaper);
	(void) signal(SIGHUP, (SIG_PF) reaper);

	ypupdated_svc_run();
	_msgout("svc_run returned");
	exit(1);
	/* NOTREACHED */
}
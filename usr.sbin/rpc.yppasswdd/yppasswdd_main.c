
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

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h> /* strcmp */
#include <syslog.h>
#include <unistd.h>

#include <rpc/rpc.h>
#include <rpc/rpc_com.h>
#include <rpc/pmap_clnt.h> /* for pmap_unset */
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "yppasswd.h"
#include "yppasswdd_extern.h"
#include "yppasswd_private.h"
#include "ypxfr_extern.h"
#include "yp_extern.h"

#ifndef SIG_PF
#define	SIG_PF void(*)(int)
#endif

#ifdef DEBUG
#define	RPC_SVC_FG
#endif

#define	_RPCSVC_CLOSEDOWN 120
int _rpcpmstart = 0;		/* Started by a port monitor ? */
static int _rpcfdtype;
		 /* Whether Stream or Datagram ? */
	/* States a server can be in wrt request */

#define	_IDLE 0
#define	_SERVED 1
#define	_SERVING 2

static char _localhost[] = "localhost";
static char _passwd_byname[] = "passwd.byname";
extern int _rpcsvcstate;	 /* Set when a request is serviced */
static char _progname[] = "rpc.yppasswdd";
char *progname = _progname;
static char _yp_dir[] = _PATH_YP;
char *yp_dir = _yp_dir;
static char _passfile_default[] = _PATH_YP "master.passwd";
char *passfile_default = _passfile_default;
char *passfile;
char *yppasswd_domain = NULL;
int no_chsh = 0;
int no_chfn = 0;
int allow_additions = 0;
int multidomain = 0;
int verbose = 0;
int resvport = 1;
int inplace = 0;
char sockname[] = YP_SOCKNAME;

static void
terminate(int sig __unused)
{
	rpcb_unset(YPPASSWDPROG, YPPASSWDVERS, NULL);
	rpcb_unset(MASTER_YPPASSWDPROG, MASTER_YPPASSWDVERS, NULL);
	unlink(sockname);
	exit(0);
}

static void
reload(int sig __unused)
{
	load_securenets();
}

static void
closedown(int sig __unused)
{
	if (_rpcsvcstate == _IDLE) {
		extern fd_set svc_fdset;
		static int size;
		int i, openfd;

		if (_rpcfdtype == SOCK_DGRAM) {
			unlink(sockname);
			exit(0);
		}
		if (size == 0) {
			size = getdtablesize();
		}
		for (i = 0, openfd = 0; i < size && openfd < 2; i++)
			if (FD_ISSET(i, &svc_fdset))
				openfd++;
		if (openfd <= 1) {
			unlink(sockname);
			exit(0);
		}
	}
	if (_rpcsvcstate == _SERVED)
		_rpcsvcstate = _IDLE;

	(void) signal(SIGALRM, (SIG_PF) closedown);
	(void) alarm(_RPCSVC_CLOSEDOWN/2);
}

static void
usage(void)
{
	fprintf(stderr, "%s\n%s\n",
"usage: rpc.yppasswdd [-t master.passwd file] [-d domain] [-p path] [-s]",
"                     [-f] [-m] [-i] [-a] [-v] [-u] [-h]");
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct rlimit rlim;
	SVCXPRT *transp = NULL;
	struct sockaddr_in saddr;
	socklen_t asize = sizeof (saddr);
	struct netconfig *nconf;
	struct sigaction sa;
	void *localhandle;
	int ch;
	char *mastername;
	char myname[MAXHOSTNAMELEN + 2];
	int maxrec = RPC_MAXDATASIZE;

	extern int debug;

	debug = 1;

	while ((ch = getopt(argc, argv, "t:d:p:sfamuivh")) != -1) {
		switch (ch) {
		case 't':
			passfile_default = optarg;
			break;
		case 'd':
			yppasswd_domain = optarg;
			break;
		case 's':
			no_chsh++;
			break;
		case 'f':
			no_chfn++;
			break;
		case 'p':
			yp_dir = optarg;
			break;
		case 'a':
			allow_additions++;
			break;
		case 'm':
			multidomain++;
			break;
		case 'i':
			inplace++;
			break;
		case 'v':
			verbose++;
			break;
		case 'u':
			resvport = 0;
			break;
		default:
		case 'h':
			usage();
			break;
		}
	}

	if (yppasswd_domain == NULL) {
		if (yp_get_default_domain(&yppasswd_domain)) {
			yp_error("no domain specified and system domain \
name isn't set -- aborting");
		usage();
		}
	}

	load_securenets();

	if (getrpcport(_localhost, YPPROG, YPVERS, IPPROTO_UDP) <= 0) {
		yp_error("no ypserv processes registered with local portmap");
		yp_error("this host is not an NIS server -- aborting");
		exit(1);
	}

	if ((mastername = ypxfr_get_master(yppasswd_domain,
		 _passwd_byname, _localhost, 0)) == NULL) {
		yp_error("can't get name of NIS master server for domain %s",
			 				yppasswd_domain);
		exit(1);
	}

	if (gethostname((char *)&myname, sizeof(myname)) == -1) {
		yp_error("can't get local hostname: %s", strerror(errno));
		exit(1);
	}

	if (strncasecmp(mastername, (char *)&myname, sizeof(myname))) {
		yp_error("master of %s is %s, but we are %s",
			"passwd.byname", mastername, myname);
		yp_error("this host is not the NIS master server for \
the %s domain -- aborting", yppasswd_domain);
		exit(1);
	}

	debug = 0;

	if (getsockname(0, (struct sockaddr *)&saddr, &asize) == 0) {
		socklen_t ssize = sizeof (int);
		if (saddr.sin_family != AF_INET)
			exit(1);
		if (getsockopt(0, SOL_SOCKET, SO_TYPE,
		    (char *)&_rpcfdtype, &ssize) == -1)
			exit(1);
		_rpcpmstart = 1;
	}

	if (!debug && _rpcpmstart == 0) {
		if (daemon(0,0)) {
			err(1,"cannot fork");
		}
	}
	openlog("rpc.yppasswdd", LOG_PID, LOG_DAEMON);
	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_NOCLDWAIT;
	sigaction(SIGCHLD, &sa, NULL);

	rpcb_unset(YPPASSWDPROG, YPPASSWDVERS, NULL);
	rpcb_unset(MASTER_YPPASSWDPROG, MASTER_YPPASSWDVERS, NULL);

	rpc_control(RPC_SVC_CONNMAXREC_SET, &maxrec);

	if (svc_create(yppasswdprog_1, YPPASSWDPROG, YPPASSWDVERS, "netpath") == 0) {
		yp_error("cannot create yppasswd service.");
		exit(1);
	}
	if (svc_create(master_yppasswdprog_1, MASTER_YPPASSWDPROG,
	    MASTER_YPPASSWDVERS, "netpath") == 0) {
		yp_error("cannot create master_yppasswd service.");
		exit(1);
	}

	nconf = NULL;
	localhandle = setnetconfig();
	while ((nconf = getnetconfig(localhandle)) != NULL) {
		if (nconf->nc_protofmly != NULL &&
		    strcmp(nconf->nc_protofmly, NC_LOOPBACK) == 0)
			break;
	}
	if (nconf == NULL) {
		yp_error("getnetconfigent unix: %s", nc_sperror());
		exit(1);
	}
	unlink(sockname);
	transp = svcunix_create(RPC_ANYSOCK, 0, 0, sockname);
	if (transp == NULL) {
		yp_error("cannot create AF_LOCAL service.");
		exit(1);
	}
	if (!svc_reg(transp, MASTER_YPPASSWDPROG, MASTER_YPPASSWDVERS,
	    master_yppasswdprog_1, nconf)) {
		yp_error("unable to register (MASTER_YPPASSWDPROG, \
		    MASTER_YPPASSWDVERS, unix).");
		exit(1);
	}
	endnetconfig(localhandle);

	/* Only root may connect() to the AF_UNIX link. */
	if (chmod(sockname, 0))
		err(1, "chmod of %s failed", sockname);

	if (transp == (SVCXPRT *)NULL) {
		yp_error("could not create a handle");
		exit(1);
	}
	if (_rpcpmstart) {
		(void) signal(SIGALRM, (SIG_PF) closedown);
		(void) alarm(_RPCSVC_CLOSEDOWN/2);
	}

	/* Unlimited resource limits. */
	rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
	(void)setrlimit(RLIMIT_CPU, &rlim);
	(void)setrlimit(RLIMIT_FSIZE, &rlim);
	(void)setrlimit(RLIMIT_STACK, &rlim);
	(void)setrlimit(RLIMIT_DATA, &rlim);
	(void)setrlimit(RLIMIT_RSS, &rlim);

	/* Don't drop core (not really necessary, but GP's). */
	rlim.rlim_cur = rlim.rlim_max = 0;
	(void)setrlimit(RLIMIT_CORE, &rlim);

	/* Turn off signals. */
	(void)signal(SIGALRM, SIG_IGN);
	(void)signal(SIGHUP, (SIG_PF) reload);
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGPIPE, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGTERM, (SIG_PF) terminate);

	svc_run();
	yp_error("svc_run returned");
	exit(1);
	/* NOTREACHED */
}

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
static const char copyright[] =
"@(#) Copyright (c) 1985, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static const char sccsid[] = "@(#)main.c	8.1 (Berkeley) 6/5/93";
#endif /* not lint */

/*
 * IPX Routing Information Protocol Daemon
 */
#include "defs.h"
#include <sys/time.h>

#include <net/if.h>

#include <errno.h>
#include <nlist.h>
#include <signal.h>
#include <paths.h>
#include <stdlib.h>
#include <unistd.h>

#define SAP_PKT		0
#define RIP_PKT		1

struct	sockaddr_ipx addr;	/* Daemon's Address */
int	ripsock;		/* RIP Socket to listen on */
int	sapsock;		/* SAP Socket to listen on */
int	kmem;
int	install;		/* if 1 call kernel */
int	lookforinterfaces;	/* if 1 probe kernel for new up interfaces */
int	performnlist;		/* if 1 check if /kernel has changed */
int	externalinterfaces;	/* # of remote and local interfaces */
int	timeval;		/* local idea of time */
int	noteremoterequests;	/* squawk on requests from non-local nets */
int	r;			/* Routing socket to install updates with */
struct	sockaddr_ipx ipx_netmask;	/* Used in installing routes */

char	packet[MAXRXPACKETSIZE+1];

char	**argv0;

int	supplier = -1;		/* process should supply updates */
int	dosap = 1;		/* By default do SAP services. */
int	dobcast = 1;		/* A RIP/SAP broadcast is needed. */
time_t	lastbcast;		/* Time of last RIP/SAP broadcast */

struct	rip *msg = (struct rip *) &packet[sizeof (struct ipx)]; 
struct	sap_packet *sap_msg = 
		(struct sap_packet *) &packet[sizeof (struct ipx)]; 
void	hup(), fkexit(), timer();
void	process(int fd, int pkt_type);
int	getsocket(int type, int proto, struct sockaddr_ipx *sipx);
void	getinfo();
void	catchtimer();

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int nfds;
	fd_set fdvar;
	time_t ttime;
	struct itimerval tval;

	argv0 = argv;
	argv++, argc--;
	while (argc > 0 && **argv == '-') {
		if (strcmp(*argv, "-s") == 0) {
			supplier = 1;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-q") == 0) {
			supplier = 0;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-R") == 0) {
			noteremoterequests++;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-S") == 0) {
			dosap = 0;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-t") == 0) {
			tracepackets++;
			argv++, argc--;
			ftrace = stderr;
			tracing = 1; 
			continue;
		}
		if (strcmp(*argv, "-g") == 0) {
			gateway = 1;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-l") == 0) {
			gateway = -1;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-N") == 0) {
			dognreply = 0;
			argv++, argc--;
			continue;
		}
		fprintf(stderr,
			"usage: ipxrouted [ -s ] [ -q ] [ -t ] [ -g ] [ -l ] [ -N ]\n");
		exit(1);
	}
	
	
#ifndef DEBUG
	if (!tracepackets)
		daemon(0, 0);
#endif
	openlog("IPXrouted", LOG_PID, LOG_DAEMON);

	addr.sipx_family = AF_IPX;
	addr.sipx_len = sizeof(addr);
	addr.sipx_port = htons(IPXPORT_RIP);
	ipx_anynet.s_net[0] = ipx_anynet.s_net[1] = -1;
	ipx_netmask.sipx_addr.x_net = ipx_anynet;
	ipx_netmask.sipx_len = 6;
	ipx_netmask.sipx_family = AF_IPX;
	r = socket(AF_ROUTE, SOCK_RAW, 0);
	/* later, get smart about lookingforinterfaces */
	if (r)
		shutdown(r, SHUT_RD); /* for now, don't want reponses */
	else {
		fprintf(stderr, "IPXrouted: no routing socket\n");
		exit(1);
	}
	ripsock = getsocket(SOCK_DGRAM, 0, &addr);
	if (ripsock < 0)
		exit(1);

	if (dosap) {
		addr.sipx_port = htons(IPXPORT_SAP);
		sapsock = getsocket(SOCK_DGRAM, 0, &addr);
		if (sapsock < 0)
			exit(1);
	} else
		sapsock = -1;

	/*
	 * Any extra argument is considered
	 * a tracing log file.
	 */
	if (argc > 0)
		traceon(*argv);
	/*
	 * Collect an initial view of the world by
	 * snooping in the kernel.  Then, send a request packet on all
	 * directly connected networks to find out what
	 * everyone else thinks.
	 */
	rtinit();
	sapinit();
	ifinit();
	if (supplier < 0)
		supplier = 0;
	/* request the state of the world */
	msg->rip_cmd = htons(RIPCMD_REQUEST);
	msg->rip_nets[0].rip_dst = ipx_anynet;
	msg->rip_nets[0].rip_metric =  htons(HOPCNT_INFINITY);
	msg->rip_nets[0].rip_ticks =  htons(-1);
	toall(sndmsg, NULL, 0);

	if (dosap) {
		sap_msg->sap_cmd = htons(SAP_REQ);
		sap_msg->sap[0].ServType = htons(SAP_WILDCARD);
		toall(sapsndmsg, NULL, 0);
	}

	signal(SIGALRM, catchtimer);
	signal(SIGHUP, hup);
	signal(SIGINT, hup);
	signal(SIGEMT, fkexit);
	signal(SIGINFO, getinfo);
 
	tval.it_interval.tv_sec = TIMER_RATE;
	tval.it_interval.tv_usec = 0;
	tval.it_value.tv_sec = TIMER_RATE;
	tval.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &tval, NULL);

	nfds = 1 + max(sapsock, ripsock);

	for (;;) {
		if (dobcast) {
			dobcast = 0;
			lastbcast = time(NULL);
			timer();
		}

		FD_ZERO(&fdvar);
		if (dosap) {
			FD_SET(sapsock, &fdvar);
		}
		FD_SET(ripsock, &fdvar);

		if(select(nfds, &fdvar, (fd_set *)NULL, (fd_set *)NULL, 
		   (struct timeval *)NULL) < 0) {
			if(errno == EINTR)
				continue;
			perror("during select");
			exit(1);
		}

		if(FD_ISSET(ripsock, &fdvar))
			process(ripsock, RIP_PKT);

		if(dosap && FD_ISSET(sapsock, &fdvar))
			process(sapsock, SAP_PKT);

		ttime = time(NULL);
		if (ttime > (lastbcast + TIMER_RATE + (TIMER_RATE * 2 / 3))) {
			dobcast = 1;
			syslog(LOG_ERR, "Missed alarm");
		}
	}
}

void
process(fd, pkt_type)
	int fd;
	int pkt_type;
{
	struct sockaddr from;
	int  cc, omask;
	socklen_t fromlen = sizeof (from);
	struct ipx *ipxdp = (struct ipx *)packet;

	cc = recvfrom(fd, packet, sizeof (packet), 0, &from, &fromlen);
	if (cc <= 0) {
		if (cc < 0 && errno != EINTR)
			syslog(LOG_ERR, "recvfrom: %m");
		return;
	}
	if (tracepackets > 1 && ftrace) {
	    fprintf(ftrace,"rcv %d bytes on %s ", 
		    cc, ipxdp_ntoa(&ipxdp->ipx_dna));
	    fprintf(ftrace," from %s\n", ipxdp_ntoa(&ipxdp->ipx_sna));
	}
	
	if (noteremoterequests && 
	    !ipx_neteqnn(ipxdp->ipx_sna.x_net, ipx_zeronet) &&
	    !ipx_neteq(ipxdp->ipx_sna, ipxdp->ipx_dna))
	{
		syslog(LOG_ERR,
		       "net of interface (%s) != net on ether (%s)!\n",
		       ipxdp_nettoa(ipxdp->ipx_dna.x_net),
		       ipxdp_nettoa(ipxdp->ipx_sna.x_net));
	}
			
	/* We get the IPX header in front of the RIF packet*/
	cc -= sizeof (struct ipx);
#define	mask(s)	(1<<((s)-1))
	omask = sigblock(mask(SIGALRM));
	switch(pkt_type) {
		case SAP_PKT: sap_input(&from, cc);
				break;
		case RIP_PKT: rip_input(&from, cc);
				break;
	}
	sigsetmask(omask);
}

int
getsocket(type, proto, sipx)
	int type, proto; 
	struct sockaddr_ipx *sipx;
{
	int domain = sipx->sipx_family;
	int retry, s, on = 1;

	retry = 1;
	while ((s = socket(domain, type, proto)) < 0 && retry) {
		syslog(LOG_ERR, "socket: %m");
		sleep(5 * retry);
		retry <<= 1;
	}
	if (retry == 0)
		return (-1);
	while (bind(s, (struct sockaddr *)sipx, sizeof (*sipx)) < 0 && retry) {
		syslog(LOG_ERR, "bind: %m");
		sleep(5 * retry);
		retry <<= 1;
	}
	if (retry == 0)
		return (-1);
	if (domain==AF_IPX) {
		struct ipx ipxdp;
		if (setsockopt(s, 0, SO_HEADERS_ON_INPUT, &on, sizeof(on))) {
			syslog(LOG_ERR, "setsockopt SEE HEADERS: %m");
			exit(1);
		}
		if (ntohs(sipx->sipx_addr.x_port) == IPXPORT_RIP)
			ipxdp.ipx_pt = IPXPROTO_RI;
		else if (ntohs(sipx->sipx_addr.x_port) == IPXPORT_SAP)
#ifdef IPXPROTO_SAP
			ipxdp.ipx_pt = IPXPROTO_SAP;
#else
			ipxdp.ipx_pt = IPXPROTO_PXP;
#endif
		else {
			syslog(LOG_ERR, "port should be either RIP or SAP");
			exit(1);
		}
		if (setsockopt(s, 0, SO_DEFAULT_HEADERS, &ipxdp, sizeof(ipxdp))) {
			syslog(LOG_ERR, "setsockopt SET HEADER: %m");
			exit(1);
		}
	}
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
		exit(1);
	}
	return (s);
}

/*
 * Fork and exit on EMT-- for profiling.
 */
void
fkexit()
{
	if (fork() == 0)
		exit(0);
}

void
catchtimer()
{
	dobcast = 1;
}

void
getinfo()
{
	FILE *fh;

	fh = fopen("/var/log/ipxrouted.dmp", "a");
	if(fh == NULL)
		return;

	dumpriptable(fh);
	dumpsaptable(fh, sap_head);

	fclose(fh);
}
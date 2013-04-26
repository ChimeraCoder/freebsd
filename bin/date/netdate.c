
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)netdate.c	8.1 (Berkeley) 5/31/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#define	WAITACK		2	/* seconds */
#define	WAITDATEACK	5	/* seconds */

/*
 * Set the date in the machines controlled by timedaemons by communicating the
 * new date to the local timedaemon.  If the timedaemon is in the master state,
 * it performs the correction on all slaves.  If it is in the slave state, it
 * notifies the master that a correction is needed.
 * Returns 0 on success.  Returns > 0 on failure, setting retval to 2;
 */
int
netsettime(time_t tval)
{
	struct timeval tout;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in lsin, dest, from;
	fd_set ready;
	long waittime;
	int s, port, timed_ack, found, lerr;
	socklen_t length;
	char hostname[MAXHOSTNAMELEN];

	if ((sp = getservbyname("timed", "udp")) == NULL) {
		warnx("timed/udp: unknown service");
		return (retval = 2);
	}

	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EAFNOSUPPORT)
			warn("timed");
		return (retval = 2);
	}

	memset(&lsin, 0, sizeof(lsin));
	lsin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		lsin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&lsin, sizeof(lsin)) >= 0)
			break;
		if (errno == EADDRINUSE)
			continue;
		if (errno != EADDRNOTAVAIL)
			warn("bind");
		goto bad;
	}
	if (port == IPPORT_RESERVED / 2) {
		warnx("all ports in use");
		goto bad;
	}
	memset(&msg, 0, sizeof(msg));
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	if (gethostname(hostname, sizeof(hostname))) {
		warn("gethostname");
		goto bad;
	}
	(void)strlcpy(msg.tsp_name, hostname, sizeof(msg.tsp_name));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)tval);
	msg.tsp_time.tv_usec = htonl((u_long)0);
	length = sizeof(struct sockaddr_in);
	if (connect(s, (struct sockaddr *)&dest, length) < 0) {
		warn("connect");
		goto bad;
	}
	if (send(s, (char *)&msg, sizeof(struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			warn("send");
		goto bad;
	}

	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;

	FD_ZERO(&ready);
	FD_SET(s, &ready);
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);

	length = sizeof(lerr);
	if (!getsockopt(s,
	    SOL_SOCKET, SO_ERROR, (char *)&lerr, &length) && lerr) {
		if (lerr != ECONNREFUSED)
			warnc(lerr, "send (delayed error)");
		goto bad;
	}

	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof(struct sockaddr_in);
		if (recvfrom(s, &msg, sizeof(struct tsp), 0,
		    (struct sockaddr *)&from, &length) < 0) {
			if (errno != ECONNREFUSED)
				warn("recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);
		switch (msg.tsp_type) {
		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;
		case TSP_DATEACK:
			(void)close(s);
			return (0);
		default:
			warnx("wrong ack received from timed: %s",
			    tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1)
		warnx("can't reach time daemon, time set locally");

bad:
	(void)close(s);
	return (retval = 2);
}
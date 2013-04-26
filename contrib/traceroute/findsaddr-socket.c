
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

/* XXX Yes this is WAY too complicated */

#ifndef lint
static const char rcsid[] =
    "@(#) $Id: findsaddr-socket.c,v 1.1 2000/11/23 20:17:12 leres Exp $ (LBL)";
#endif

#include <sys/param.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#include <sys/time.h>				/* concession to AIX */

#if __STDC__
struct mbuf;
struct rtentry;
#endif

#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <netinet/in.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gnuc.h"
#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#include "findsaddr.h"

#ifdef HAVE_SOCKADDR_SA_LEN
#define SALEN(sa) ((sa)->sa_len)
#else
#define SALEN(sa) salen(sa)
#endif

#ifndef roundup
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */
#endif

struct rtmsg {
        struct rt_msghdr rtmsg;
        u_char data[512];
};

static struct rtmsg rtmsg = {
	{ 0, RTM_VERSION, RTM_GET, 0,
	RTF_UP | RTF_GATEWAY | RTF_HOST | RTF_STATIC,
	RTA_DST | RTA_IFA, 0, 0, 0, 0, 0, { 0 } },
	{ 0 }
};

#ifndef HAVE_SOCKADDR_SA_LEN
static int salen(struct sockaddr *);
#endif

/*
 * Return the source address for the given destination address
 */
const char *
findsaddr(register const struct sockaddr_in *to,
    register struct sockaddr_in *from)
{
	register struct rt_msghdr *rp;
	register u_char *cp;

	register struct sockaddr_in *sp, *ifa;
	register struct sockaddr *sa;
	register int s, size, cc, seq, i;
	register pid_t pid;
	static char errbuf[512];

	s = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
	if (s < 0) {
		sprintf(errbuf, "socket: %.128s", strerror(errno));
		return (errbuf);
	}

	seq = 0;
	pid = getpid();

	rp = &rtmsg.rtmsg;
	rp->rtm_seq = ++seq;
	cp = (u_char *)(rp + 1);

	sp = (struct sockaddr_in *)cp;
	*sp = *to;
	cp += roundup(SALEN((struct sockaddr *)sp), sizeof(u_int32_t));

	size = cp - (u_char *)rp;
	rp->rtm_msglen = size;

	cc = write(s, (char *)rp, size);
	if (cc < 0) {
		sprintf(errbuf, "write: %.128s", strerror(errno));
		close(s);
		return (errbuf);
	}
	if (cc != size) {
		sprintf(errbuf, "short write (%d != %d)", cc, size);
		close(s);
		return (errbuf);
	}

	size = sizeof(rtmsg);
	do {
		memset(rp, 0, size);
		cc = read(s, (char *)rp, size);
		if (cc < 0) {
			sprintf(errbuf, "read: %.128s", strerror(errno));
			close(s);
			return (errbuf);
		}

	} while (rp->rtm_seq != seq || rp->rtm_pid != pid);
	close(s);


	if (rp->rtm_version != RTM_VERSION) {
		sprintf(errbuf, "bad version %d", rp->rtm_version);
		return (errbuf);
	}
	if (rp->rtm_msglen > cc) {
		sprintf(errbuf, "bad msglen %d > %d", rp->rtm_msglen, cc);
		return (errbuf);
	}
	if (rp->rtm_errno != 0) {
		sprintf(errbuf, "rtm_errno: %.128s", strerror(rp->rtm_errno));
		return (errbuf);
	}

	/* Find the interface sockaddr */
	cp = (u_char *)(rp + 1);
	for (i = 1; i != 0; i <<= 1)
		if ((i & rp->rtm_addrs) != 0) {
			sa = (struct sockaddr *)cp;
			switch (i) {

			case RTA_IFA:
				if (sa->sa_family == AF_INET) {
					ifa = (struct sockaddr_in *)cp;
					if (ifa->sin_addr.s_addr != 0) {
						*from = *ifa;
						return (NULL);
					}
				}
				break;

			}

			if (SALEN(sa) == 0)
				cp += sizeof(long);
			else
				cp += roundup(SALEN(sa), sizeof(long));
		}

	return ("failed!");
}

#ifndef HAVE_SOCKADDR_SA_LEN
static int
salen(struct sockaddr *sa)
{
	switch (sa->sa_family) {

	case AF_INET:
		return (sizeof(struct sockaddr_in));

	case AF_LINK:
		return (sizeof(struct sockaddr_dl));

	default:
		return (sizeof(struct sockaddr));
	}
}
#endif
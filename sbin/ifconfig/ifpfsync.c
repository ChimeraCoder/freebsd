
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

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <net/if.h>
#include <netinet/in.h>
#include <net/pfvar.h>
#include <net/if_pfsync.h>
#include <net/route.h>
#include <arpa/inet.h>

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ifconfig.h"

void setpfsync_syncdev(const char *, int, int, const struct afswtch *);
void unsetpfsync_syncdev(const char *, int, int, const struct afswtch *);
void setpfsync_syncpeer(const char *, int, int, const struct afswtch *);
void unsetpfsync_syncpeer(const char *, int, int, const struct afswtch *);
void setpfsync_syncpeer(const char *, int, int, const struct afswtch *);
void setpfsync_maxupd(const char *, int, int, const struct afswtch *);
void setpfsync_defer(const char *, int, int, const struct afswtch *);
void pfsync_status(int);

void
setpfsync_syncdev(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct pfsyncreq preq;

	bzero((char *)&preq, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETPFSYNC");

	strlcpy(preq.pfsyncr_syncdev, val, sizeof(preq.pfsyncr_syncdev));

	if (ioctl(s, SIOCSETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETPFSYNC");
}

/* ARGSUSED */
void
unsetpfsync_syncdev(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct pfsyncreq preq;

	bzero((char *)&preq, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETPFSYNC");

	bzero((char *)&preq.pfsyncr_syncdev, sizeof(preq.pfsyncr_syncdev));

	if (ioctl(s, SIOCSETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETPFSYNC");
}

/* ARGSUSED */
void
setpfsync_syncpeer(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct pfsyncreq preq;
	struct addrinfo hints, *peerres;
	int ecode;

	bzero((char *)&preq, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETPFSYNC");

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;	/*dummy*/

	if ((ecode = getaddrinfo(val, NULL, &hints, &peerres)) != 0)
		errx(1, "error in parsing address string: %s",
		    gai_strerror(ecode));

	if (peerres->ai_addr->sa_family != AF_INET)
		errx(1, "only IPv4 addresses supported for the syncpeer");

	preq.pfsyncr_syncpeer.s_addr = ((struct sockaddr_in *)
	    peerres->ai_addr)->sin_addr.s_addr;

	if (ioctl(s, SIOCSETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETPFSYNC");
}

/* ARGSUSED */
void
unsetpfsync_syncpeer(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct pfsyncreq preq;

	bzero((char *)&preq, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETPFSYNC");

	preq.pfsyncr_syncpeer.s_addr = 0;

	if (ioctl(s, SIOCSETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETPFSYNC");
}

/* ARGSUSED */
void
setpfsync_maxupd(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct pfsyncreq preq;
	int maxupdates;

	maxupdates = atoi(val);
	if ((maxupdates < 0) || (maxupdates > 255))
		errx(1, "maxupd %s: out of range", val);

	memset((char *)&preq, 0, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETPFSYNC");

	preq.pfsyncr_maxupdates = maxupdates;

	if (ioctl(s, SIOCSETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETPFSYNC");
}

/* ARGSUSED */
void
setpfsync_defer(const char *val, int d, int s, const struct afswtch *rafp)
{
	struct pfsyncreq preq;

	memset((char *)&preq, 0, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCGETPFSYNC");

	preq.pfsyncr_defer = d;
	if (ioctl(s, SIOCSETPFSYNC, (caddr_t)&ifr) == -1)
		err(1, "SIOCSETPFSYNC");
}

void
pfsync_status(int s)
{
	struct pfsyncreq preq;

	bzero((char *)&preq, sizeof(struct pfsyncreq));
	ifr.ifr_data = (caddr_t)&preq;

	if (ioctl(s, SIOCGETPFSYNC, (caddr_t)&ifr) == -1)
		return;

	if (preq.pfsyncr_syncdev[0] != '\0' ||
	    preq.pfsyncr_syncpeer.s_addr != INADDR_PFSYNC_GROUP)
			printf("\t");

	if (preq.pfsyncr_syncdev[0] != '\0')
		printf("pfsync: syncdev: %s ", preq.pfsyncr_syncdev);
	if (preq.pfsyncr_syncpeer.s_addr != INADDR_PFSYNC_GROUP)
		printf("syncpeer: %s ", inet_ntoa(preq.pfsyncr_syncpeer));

	if (preq.pfsyncr_syncdev[0] != '\0' ||
	    preq.pfsyncr_syncpeer.s_addr != INADDR_PFSYNC_GROUP) {
		printf("maxupd: %d ", preq.pfsyncr_maxupdates);
		printf("defer: %s\n", preq.pfsyncr_defer ? "on" : "off");
	}
}

static struct cmd pfsync_cmds[] = {
	DEF_CMD_ARG("syncdev",		setpfsync_syncdev),
	DEF_CMD("-syncdev",	1,	unsetpfsync_syncdev),
	DEF_CMD_ARG("syncif",		setpfsync_syncdev),
	DEF_CMD("-syncif",	1,	unsetpfsync_syncdev),
	DEF_CMD_ARG("syncpeer",		setpfsync_syncpeer),
	DEF_CMD("-syncpeer",	1,	unsetpfsync_syncpeer),
	DEF_CMD_ARG("maxupd",		setpfsync_maxupd),
	DEF_CMD("defer",	1,	setpfsync_defer),
	DEF_CMD("-defer",	0,	setpfsync_defer),
};
static struct afswtch af_pfsync = {
	.af_name	= "af_pfsync",
	.af_af		= AF_UNSPEC,
	.af_other_status = pfsync_status,
};

static __constructor void
pfsync_ctor(void)
{
#define	N(a)	(sizeof(a) / sizeof(a[0]))
	int i;

	for (i = 0; i < N(pfsync_cmds);  i++)
		cmd_register(&pfsync_cmds[i]);
	af_register(&af_pfsync);
#undef N
}
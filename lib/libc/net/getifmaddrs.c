
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

#include "namespace.h"
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>

#include <errno.h>
#include <ifaddrs.h>
#include <stdlib.h>
#include <string.h>
#include "un-namespace.h"

#define	SALIGN	(sizeof(long) - 1)
#define	SA_RLEN(sa)	((sa)->sa_len ? (((sa)->sa_len + SALIGN) & ~SALIGN) : \
			    (SALIGN + 1))
#define	MAX_SYSCTL_TRY	5
#define	RTA_MASKS	(RTA_GATEWAY | RTA_IFP | RTA_IFA)

int
getifmaddrs(struct ifmaddrs **pif)
{
	int icnt = 1;
	int dcnt = 0;
	int ntry = 0;
	size_t len;
	size_t needed;
	int mib[6];
	int i;
	char *buf;
	char *data;
	char *next;
	char *p;
	struct ifma_msghdr *ifmam;
	struct ifmaddrs *ifa, *ift;
	struct rt_msghdr *rtm;
	struct sockaddr *sa;

	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;             /* protocol */
	mib[3] = 0;             /* wildcard address family */
	mib[4] = NET_RT_IFMALIST;
	mib[5] = 0;             /* no flags */
	do {
		if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
			return (-1);
		if ((buf = malloc(needed)) == NULL)
			return (-1);
		if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
			if (errno != ENOMEM || ++ntry >= MAX_SYSCTL_TRY) {
				free(buf);
				return (-1);
			}
			free(buf);
			buf = NULL;
		} 
	} while (buf == NULL);

	for (next = buf; next < buf + needed; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)(void *)next;
		if (rtm->rtm_version != RTM_VERSION)
			continue;
		switch (rtm->rtm_type) {
		case RTM_NEWMADDR:
			ifmam = (struct ifma_msghdr *)(void *)rtm;
			if ((ifmam->ifmam_addrs & RTA_IFA) == 0)
				break;
			icnt++;
			p = (char *)(ifmam + 1);
			for (i = 0; i < RTAX_MAX; i++) {
				if ((RTA_MASKS & ifmam->ifmam_addrs &
				    (1 << i)) == 0)
					continue;
				sa = (struct sockaddr *)(void *)p;
				len = SA_RLEN(sa);
				dcnt += len;
				p += len;
			}
			break;
		}
	}

	data = malloc(sizeof(struct ifmaddrs) * icnt + dcnt);
	if (data == NULL) {
		free(buf);
		return (-1);
	}

	ifa = (struct ifmaddrs *)(void *)data;
	data += sizeof(struct ifmaddrs) * icnt;

	memset(ifa, 0, sizeof(struct ifmaddrs) * icnt);
	ift = ifa;

	for (next = buf; next < buf + needed; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)(void *)next;
		if (rtm->rtm_version != RTM_VERSION)
			continue;

		switch (rtm->rtm_type) {
		case RTM_NEWMADDR:
			ifmam = (struct ifma_msghdr *)(void *)rtm;
			if ((ifmam->ifmam_addrs & RTA_IFA) == 0)
				break;

			p = (char *)(ifmam + 1);
			for (i = 0; i < RTAX_MAX; i++) {
				if ((RTA_MASKS & ifmam->ifmam_addrs &
				    (1 << i)) == 0)
					continue;
				sa = (struct sockaddr *)(void *)p;
				len = SA_RLEN(sa);
				switch (i) {
				case RTAX_GATEWAY:
					ift->ifma_lladdr =
					    (struct sockaddr *)(void *)data;
					memcpy(data, p, len);
					data += len;
					break;

				case RTAX_IFP:
					ift->ifma_name =
					    (struct sockaddr *)(void *)data;
					memcpy(data, p, len);
					data += len;
					break;

				case RTAX_IFA:
					ift->ifma_addr =
					    (struct sockaddr *)(void *)data;
					memcpy(data, p, len);
					data += len;
					break;

				default:
					data += len;
					break;
				}
				p += len;
			}
			ift->ifma_next = ift + 1;
			ift = ift->ifma_next;
			break;
		}
	}

	free(buf);

	if (ift > ifa) {
		ift--;
		ift->ifma_next = NULL;
		*pif = ifa;
	} else {
		*pif = NULL;
		free(ifa);
	}
	return (0);
}

void
freeifmaddrs(struct ifmaddrs *ifmp)
{

	free(ifmp);
}
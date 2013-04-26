
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
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/file.h>
#include <sys/filedesc.h>
#include <sys/sockio.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/vnet.h>

#include <compat/svr4/svr4.h>
#include <compat/svr4/svr4_util.h>
#include <compat/svr4/svr4_ioctl.h>
#include <compat/svr4/svr4_sockio.h>

static int bsd_to_svr4_flags(int);

#define bsd_to_svr4_flag(a) \
	if (bf & __CONCAT(I,a))	sf |= __CONCAT(SVR4_I,a)

static int
bsd_to_svr4_flags(bf)
	int bf;
{
	int sf = 0;
	bsd_to_svr4_flag(FF_UP);
	bsd_to_svr4_flag(FF_BROADCAST);
	bsd_to_svr4_flag(FF_DEBUG);
	bsd_to_svr4_flag(FF_LOOPBACK);
	bsd_to_svr4_flag(FF_POINTOPOINT);
#if defined(IFF_NOTRAILERS)
	bsd_to_svr4_flag(FF_NOTRAILERS);
#endif
	if (bf & IFF_DRV_RUNNING)
		sf |= SVR4_IFF_RUNNING;
	bsd_to_svr4_flag(FF_NOARP);
	bsd_to_svr4_flag(FF_PROMISC);
	bsd_to_svr4_flag(FF_ALLMULTI);
	bsd_to_svr4_flag(FF_MULTICAST);
	return sf;
}

int
svr4_sock_ioctl(fp, td, retval, fd, cmd, data)
	struct file *fp;
	struct thread *td;
	register_t *retval;
	int fd;
	u_long cmd;
	caddr_t data;
{
	int error;

	*retval = 0;

	switch (cmd) {
	case SVR4_SIOCGIFNUM:
		{
			struct ifnet *ifp;
			struct ifaddr *ifa;
			int ifnum = 0;

			/*
			 * This does not return the number of physical
			 * interfaces (if_index), but the number of interfaces
			 * + addresses like ifconf() does, because this number
			 * is used by code that will call SVR4_SIOCGIFCONF to
			 * find the space needed for SVR4_SIOCGIFCONF. So we
			 * count the number of ifreq entries that the next
			 * SVR4_SIOCGIFCONF will return. Maybe a more correct
			 * fix is to make SVR4_SIOCGIFCONF return only one
			 * entry per physical interface?
			 */
			IFNET_RLOCK();
			TAILQ_FOREACH(ifp, &V_ifnet, if_link)
				if (TAILQ_EMPTY(&ifp->if_addrhead))
					ifnum++;
				else
					TAILQ_FOREACH(ifa, &ifp->if_addrhead,
					    ifa_link)
						ifnum++;
			IFNET_RUNLOCK();
			DPRINTF(("SIOCGIFNUM %d\n", ifnum));
			return copyout(&ifnum, data, sizeof(ifnum));
		}

	case SVR4_SIOCGIFFLAGS:
		{
			struct ifreq br;
			struct svr4_ifreq sr;

			if ((error = copyin(data, &sr, sizeof(sr))) != 0)
				return error;

			(void) strlcpy(br.ifr_name, sr.svr4_ifr_name,
			    sizeof(br.ifr_name));
			if ((error = fo_ioctl(fp, SIOCGIFFLAGS, 
					    (caddr_t) &br, td->td_ucred,
					    td)) != 0) {
				DPRINTF(("SIOCGIFFLAGS (%s) %s: error %d\n", 
					 br.ifr_name, sr.svr4_ifr_name, error));
				return error;
			}

			sr.svr4_ifr_flags = bsd_to_svr4_flags(br.ifr_flags);
			DPRINTF(("SIOCGIFFLAGS %s = %x\n", 
				sr.svr4_ifr_name, sr.svr4_ifr_flags));
			return copyout(&sr, data, sizeof(sr));
		}

	case SVR4_SIOCGIFCONF:
		{
			struct svr4_ifconf sc;

			if ((error = copyin(data, &sc, sizeof(sc))) != 0)
				return error;

			DPRINTF(("ifreq %d svr4_ifreq %d ifc_len %d\n",
				sizeof(struct ifreq), sizeof(struct svr4_ifreq),
				sc.svr4_ifc_len));

			if ((error = fo_ioctl(fp, OSIOCGIFCONF,
					    (caddr_t) &sc, td->td_ucred,
					    td)) != 0)
				return error;

			DPRINTF(("SIOCGIFCONF\n"));
			return 0;
		}


	default:
		DPRINTF(("Unknown svr4 sockio %lx\n", cmd));
		return 0;	/* ENOSYS really */
	}
}
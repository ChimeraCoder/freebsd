
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

/*
 * Discard interface driver for protocol testing and timing.
 * (Based on the loopback.)
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mbuf.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <net/if.h>
#include <net/if_clone.h>
#include <net/if_types.h>
#include <net/route.h>
#include <net/bpf.h>

#include "opt_inet.h"
#include "opt_inet6.h"

#ifdef TINY_DSMTU
#define	DSMTU	(1024+512)
#else
#define DSMTU	65532
#endif

struct disc_softc {
	struct ifnet *sc_ifp;
};

static int	discoutput(struct ifnet *, struct mbuf *,
		    const struct sockaddr *, struct route *);
static void	discrtrequest(int, struct rtentry *, struct rt_addrinfo *);
static int	discioctl(struct ifnet *, u_long, caddr_t);
static int	disc_clone_create(struct if_clone *, int, caddr_t);
static void	disc_clone_destroy(struct ifnet *);

static const char discname[] = "disc";
static MALLOC_DEFINE(M_DISC, discname, "Discard interface");

static struct if_clone *disc_cloner;

static int
disc_clone_create(struct if_clone *ifc, int unit, caddr_t params)
{
	struct ifnet		*ifp;
	struct disc_softc	*sc;

	sc = malloc(sizeof(struct disc_softc), M_DISC, M_WAITOK | M_ZERO);
	ifp = sc->sc_ifp = if_alloc(IFT_LOOP);
	if (ifp == NULL) {
		free(sc, M_DISC);
		return (ENOSPC);
	}

	ifp->if_softc = sc;
	if_initname(ifp, discname, unit);
	ifp->if_mtu = DSMTU;
	/*
	 * IFF_LOOPBACK should not be removed from disc's flags because
	 * it controls what PF-specific routes are magically added when
	 * a network address is assigned to the interface.  Things just
	 * won't work as intended w/o such routes because the output
	 * interface selection for a packet is totally route-driven.
	 * A valid alternative to IFF_LOOPBACK can be IFF_BROADCAST or
	 * IFF_POINTOPOINT, but it would result in different properties
	 * of the interface.
	 */
	ifp->if_flags = IFF_LOOPBACK | IFF_MULTICAST;
	ifp->if_drv_flags = IFF_DRV_RUNNING;
	ifp->if_ioctl = discioctl;
	ifp->if_output = discoutput;
	ifp->if_hdrlen = 0;
	ifp->if_addrlen = 0;
	ifp->if_snd.ifq_maxlen = 20;
	if_attach(ifp);
	bpfattach(ifp, DLT_NULL, sizeof(u_int32_t));

	return (0);
}

static void
disc_clone_destroy(struct ifnet *ifp)
{
	struct disc_softc	*sc;

	sc = ifp->if_softc;

	bpfdetach(ifp);
	if_detach(ifp);
	if_free(ifp);

	free(sc, M_DISC);
}

static int
disc_modevent(module_t mod, int type, void *data)
{

	switch (type) {
	case MOD_LOAD:
		disc_cloner = if_clone_simple(discname, disc_clone_create,
		    disc_clone_destroy, 0);
		break;
	case MOD_UNLOAD:
		if_clone_detach(disc_cloner);
		break;
	default:
		return (EOPNOTSUPP);
	}
	return (0);
}

static moduledata_t disc_mod = {
	"if_disc",
	disc_modevent,
	NULL
};

DECLARE_MODULE(if_disc, disc_mod, SI_SUB_PSEUDO, SI_ORDER_ANY);

static int
discoutput(struct ifnet *ifp, struct mbuf *m, const struct sockaddr *dst,
    struct route *ro)
{
	u_int32_t af;

	M_ASSERTPKTHDR(m);

	/* BPF writes need to be handled specially. */
	if (dst->sa_family == AF_UNSPEC)
		bcopy(dst->sa_data, &af, sizeof(af));
	else
		af = dst->sa_family;

	if (bpf_peers_present(ifp->if_bpf))
		bpf_mtap2(ifp->if_bpf, &af, sizeof(af), m);

	m->m_pkthdr.rcvif = ifp;

	ifp->if_opackets++;
	ifp->if_obytes += m->m_pkthdr.len;

	m_freem(m);
	return (0);
}

/* ARGSUSED */
static void
discrtrequest(int cmd, struct rtentry *rt, struct rt_addrinfo *info)
{
	RT_LOCK_ASSERT(rt);
	rt->rt_rmx.rmx_mtu = DSMTU;
}

/*
 * Process an ioctl request.
 */
static int
discioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	struct ifaddr *ifa;
	struct ifreq *ifr = (struct ifreq *)data;
	int error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		ifa = (struct ifaddr *)data;
		if (ifa != 0)
			ifa->ifa_rtrequest = discrtrequest;
		/*
		 * Everything else is done at a higher level.
		 */
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		if (ifr == 0) {
			error = EAFNOSUPPORT;		/* XXX */
			break;
		}
		switch (ifr->ifr_addr.sa_family) {

#ifdef INET
		case AF_INET:
			break;
#endif
#ifdef INET6
		case AF_INET6:
			break;
#endif

		default:
			error = EAFNOSUPPORT;
			break;
		}
		break;

	case SIOCSIFMTU:
		ifp->if_mtu = ifr->ifr_mtu;
		break;

	default:
		error = EINVAL;
	}
	return (error);
}
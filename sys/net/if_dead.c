
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
 * When an interface has been detached but not yet freed, we set the various
 * ifnet function pointers to "ifdead" versions.  This prevents unexpected
 * calls from the network stack into the device driver after if_detach() has
 * returned.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/mbuf.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_var.h>

static int
ifdead_output(struct ifnet *ifp, struct mbuf *m, const struct sockaddr *sa,
    struct route *ro)
{

	m_freem(m);
	return (ENXIO);
}

static void
ifdead_input(struct ifnet *ifp, struct mbuf *m)
{

	m_freem(m);
}

static void
ifdead_start(struct ifnet *ifp)
{

}

static int
ifdead_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{

	return (ENXIO);
}

static int
ifdead_resolvemulti(struct ifnet *ifp, struct sockaddr **llsa,
    struct sockaddr *sa)
{

	*llsa = NULL;
	return (ENXIO);
}

static void
ifdead_qflush(struct ifnet *ifp)
{

}

static int
ifdead_transmit(struct ifnet *ifp, struct mbuf *m)
{

	m_freem(m);
	return (ENXIO);
}

void
if_dead(struct ifnet *ifp)
{

	ifp->if_output = ifdead_output;
	ifp->if_input = ifdead_input;
	ifp->if_start = ifdead_start;
	ifp->if_ioctl = ifdead_ioctl;
	ifp->if_resolvemulti = ifdead_resolvemulti;
	ifp->if_qflush = ifdead_qflush;
	ifp->if_transmit = ifdead_transmit;
}
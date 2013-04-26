
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
 * atm_pcb.c: manage atm protocol control blocks and keep IP and NATM
 * from trying to use each other's VCs.
 */

#include "opt_ddb.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#include <net/if.h>

#include <netinet/in.h>

#include <netnatm/natm.h>

#include <ddb/ddb.h>

struct npcblist natm_pcbs;

/*
 * npcb_alloc: allocate a npcb [in the free state]
 */
struct natmpcb *
npcb_alloc(int wait)

{
	struct natmpcb *npcb;

	npcb = malloc(sizeof(*npcb), M_PCB, wait | M_ZERO);
	if (npcb != NULL)
		npcb->npcb_flags = NPCB_FREE;
	return (npcb);
}


/*
 * npcb_free: free a npcb
 */
void
npcb_free(struct natmpcb *npcb, int op)
{

	NATM_LOCK_ASSERT();

	if ((npcb->npcb_flags & NPCB_FREE) == 0) {
		LIST_REMOVE(npcb, pcblist);
		npcb->npcb_flags = NPCB_FREE;
	}
	if (op == NPCB_DESTROY) {
		if (npcb->npcb_inq) {
			npcb->npcb_flags = NPCB_DRAIN;	/* flag for distruct. */
		} else {
			free(npcb, M_PCB);		/* kill it! */
		}
	}
}


/*
 * npcb_add: add or remove npcb from main list
 *   returns npcb if ok
 */
struct natmpcb *
npcb_add(struct natmpcb *npcb, struct ifnet *ifp, u_int16_t vci, u_int8_t vpi)
{
	struct natmpcb *cpcb = NULL;		/* current pcb */

	NATM_LOCK_ASSERT();

	/*
	 * lookup required
	 */
	LIST_FOREACH(cpcb, &natm_pcbs, pcblist)
		if (ifp == cpcb->npcb_ifp && vci == cpcb->npcb_vci &&
		    vpi == cpcb->npcb_vpi)
			break;

	/*
	 * add & something already there?
	 */
	if (cpcb) {
		cpcb = NULL;
		goto done;			/* fail */
	}
    
	/*
	 * need to allocate a pcb?
	 */
	if (npcb == NULL) {
		/* could be called from lower half */
		cpcb = npcb_alloc(M_NOWAIT);
		if (cpcb == NULL) 
			goto done;			/* fail */
	} else {
		cpcb = npcb;
	}

	cpcb->npcb_ifp = ifp;
	cpcb->ipaddr.s_addr = 0;
	cpcb->npcb_vci = vci;
	cpcb->npcb_vpi = vpi;
	cpcb->npcb_flags = NPCB_CONNECTED;

	LIST_INSERT_HEAD(&natm_pcbs, cpcb, pcblist);

done:
	return (cpcb);
}

#ifdef DDB
DB_SHOW_COMMAND(natm, db_show_natm)
{
	struct natmpcb *cpcb;

	db_printf("npcb dump:\n");
	LIST_FOREACH(cpcb, &natm_pcbs, pcblist) {
		db_printf("if=%s, vci=%d, vpi=%d, IP=0x%x, sock=%p, "
		    "flags=0x%x, inq=%d\n", cpcb->npcb_ifp->if_xname,
		    cpcb->npcb_vci, cpcb->npcb_vpi, cpcb->ipaddr.s_addr,
		    cpcb->npcb_socket, cpcb->npcb_flags, cpcb->npcb_inq);
	}
}
#endif

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

#include "opt_inet.h"
#include "opt_tcpdebug.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/protosw.h>

#define TCPSTATES		/* for logging */

#include <netinet/in_systm.h>
#include <netinet/tcp_fsm.h>

#include <netipx/ipx.h>
#include <netipx/ipx_var.h>
#include <netipx/spx.h>
#define SPXTIMERS
#include <netipx/spx_timer.h>
#define	SANAMES
#include <netipx/spx_debug.h>

#ifdef INET
#ifdef TCPDEBUG
static	int spxconsdebug = 0;
static	struct spx_debug spx_debug[SPX_NDEBUG];
static	int spx_debx;
#endif
#endif

/*
 * spx debug routines
 */
void
spx_trace(short act, u_char ostate, struct spxpcb *sp, struct spx *si,
    int req)
{
#ifdef INET
#ifdef TCPDEBUG
	u_short seq, ack, len, alo;
	int flags;
	struct spx_debug *sd = &spx_debug[spx_debx++];

	if (spx_debx == SPX_NDEBUG)
		spx_debx = 0;
	sd->sd_time = iptime();
	sd->sd_act = act;
	sd->sd_ostate = ostate;
	sd->sd_cb = (caddr_t)sp;
	if (sp != NULL)
		sd->sd_sp = *sp;
	else
		bzero((caddr_t)&sd->sd_sp, sizeof(*sp));
	if (si != NULL)
		sd->sd_si = *si;
	else
		bzero((caddr_t)&sd->sd_si, sizeof(*si));
	sd->sd_req = req;
	if (spxconsdebug == 0)
		return;
	if (ostate >= TCP_NSTATES)
		ostate = 0;
	if (act >= SA_DROP)
		act = SA_DROP;
	if (sp != NULL)
		printf("%p %s:", (void *)sp, tcpstates[ostate]);
	else
		printf("???????? ");
	printf("%s ", spxnames[act]);
	switch (act) {

	case SA_RESPOND:
	case SA_INPUT:
	case SA_OUTPUT:
	case SA_DROP:
		if (si == NULL)
			break;
		seq = si->si_seq;
		ack = si->si_ack;
		alo = si->si_alo;
		len = si->si_len;
		if (act == SA_OUTPUT) {
			seq = ntohs(seq);
			ack = ntohs(ack);
			alo = ntohs(alo);
			len = ntohs(len);
		}
#ifndef lint
#define p1(f)  { printf("%s = %x, ", "f", f); }
		p1(seq); p1(ack); p1(alo); p1(len);
#endif
		flags = si->si_cc;
		if (flags) {
			char *cp = "<";
#ifndef lint
#define pf(f) { if (flags & SPX_ ## f) { printf("%s%s", cp, "f"); cp = ","; } }
			pf(SP); pf(SA); pf(OB); pf(EM);
#else
			cp = cp;
#endif
			printf(">");
		}
#ifndef lint
#define p2(f)  { printf("%s = %x, ", "f", si->si_ ## f); }
		p2(sid);p2(did);p2(dt);p2(pt);
#endif
		ipx_printhost(&si->si_sna);
		ipx_printhost(&si->si_dna);

		if (act == SA_RESPOND) {
			printf("ipx_len = %x, ",
				((struct ipx *)si)->ipx_len);
		}
		break;

	case SA_USER:
		printf("%s", prurequests[req&0xff]);
		if ((req & 0xff) == PRU_SLOWTIMO)
			printf("<%s>", spxtimers[req>>8]);
		break;
	}
	if (sp)
		printf(" -> %s", tcpstates[sp->s_state]);
	/* print out internal state of sp !?! */
	printf("\n");
	if (sp == 0)
		return;
#ifndef lint
#define p3(f)  { printf("%s = %x, ", "f", sp->s_ ## f); }
	printf("\t"); p3(rack);p3(ralo);p3(smax);p3(flags); printf("\n");
#endif
#endif
#endif
}
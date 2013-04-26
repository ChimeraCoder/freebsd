
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
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <sys/serial.h>

#include <dev/scc/scc_bfe.h>
#include <dev/scc/scc_bus.h>

#include <dev/ic/sab82532.h>

#include "scc_if.h"

static int sab82532_bfe_attach(struct scc_softc *, int);
static int sab82532_bfe_iclear(struct scc_softc *, struct scc_chan *);
static int sab82532_bfe_ipend(struct scc_softc *);
static int sab82532_bfe_probe(struct scc_softc *);

static kobj_method_t sab82532_methods[] = {
	KOBJMETHOD(scc_attach,	sab82532_bfe_attach),
	KOBJMETHOD(scc_iclear,	sab82532_bfe_iclear),
	KOBJMETHOD(scc_ipend,	sab82532_bfe_ipend),
	KOBJMETHOD(scc_probe,	sab82532_bfe_probe),
	{ 0, 0 }
};

struct scc_class scc_sab82532_class = {
	"sab82532 class",
	sab82532_methods,
	sizeof(struct scc_softc),
	.cl_channels = SAB_NCHAN,
	.cl_class = SCC_CLASS_SAB82532,
	.cl_modes = SCC_MODE_ASYNC | SCC_MODE_BISYNC | SCC_MODE_HDLC,
	.cl_range = SAB_CHANLEN,
};

static int
sab82532_bfe_attach(struct scc_softc *sc, int reset)
{
	struct scc_bas *bas;

	bas = &sc->sc_bas;
	return (0);
}

static int
sab82532_bfe_iclear(struct scc_softc *sc, struct scc_chan *ch)
{

	return (0);
}

static int
sab82532_bfe_ipend(struct scc_softc *sc)
{
	struct scc_bas *bas;
	struct scc_chan *ch;
	int ipend;
	int c, ofs;
	uint8_t isr0, isr1;

	bas = &sc->sc_bas;
	ipend = 0;
	for (c = 0; c < SAB_NCHAN; c++) {
		ch = &sc->sc_chan[c];
		ofs = c * SAB_CHANLEN;
		mtx_lock_spin(&sc->sc_hwmtx);
		isr0 = scc_getreg(bas, ofs + SAB_ISR0);
		isr1 = scc_getreg(bas, ofs + SAB_ISR1);
		scc_barrier(bas);
		if (isr0 & SAB_ISR0_TIME) {
			while (scc_getreg(bas, ofs + SAB_STAR) & SAB_STAR_CEC)
				;
			scc_setreg(bas, ofs + SAB_CMDR, SAB_CMDR_RFRD);
			scc_barrier(bas);
		}
		mtx_unlock_spin(&sc->sc_hwmtx);

		ch->ch_ipend = 0;
		if (isr1 & SAB_ISR1_BRKT)
			ch->ch_ipend |= SER_INT_BREAK;
		if (isr0 & SAB_ISR0_RFO)
			ch->ch_ipend |= SER_INT_OVERRUN;
		if (isr0 & (SAB_ISR0_TCD|SAB_ISR0_RPF))
			ch->ch_ipend |= SER_INT_RXREADY;
		if ((isr0 & SAB_ISR0_CDSC) || (isr1 & SAB_ISR1_CSC))
			ch->ch_ipend |= SER_INT_SIGCHG;
		if (isr1 & SAB_ISR1_ALLS)
			ch->ch_ipend |= SER_INT_TXIDLE;
		ipend |= ch->ch_ipend;
	}
	return (ipend);
}

static int
sab82532_bfe_probe(struct scc_softc *sc)
{
	struct scc_bas *bas;

	bas = &sc->sc_bas;
	return (0);
}
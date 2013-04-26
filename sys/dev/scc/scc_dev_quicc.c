
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

#define __RMAN_RESOURCE_VISIBLE

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/endian.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <sys/serial.h>

#include <dev/scc/scc_bfe.h>
#include <dev/scc/scc_bus.h>

#include <dev/ic/quicc.h>

#include "scc_if.h"

#define	quicc_read2(bas, reg)		\
	bus_space_read_2((bas)->bst, (bas)->bsh, reg)

#define	quicc_write2(bas, reg, val)	\
	bus_space_write_2((bas)->bst, (bas)->bsh, reg, val)

static int quicc_bfe_attach(struct scc_softc *, int);
static int quicc_bfe_enabled(struct scc_softc *, struct scc_chan *);
static int quicc_bfe_iclear(struct scc_softc *, struct scc_chan *);
static int quicc_bfe_ipend(struct scc_softc *);
static int quicc_bfe_probe(struct scc_softc *);

static kobj_method_t quicc_methods[] = {
	KOBJMETHOD(scc_attach,	quicc_bfe_attach),
	KOBJMETHOD(scc_enabled,	quicc_bfe_enabled),
	KOBJMETHOD(scc_iclear,	quicc_bfe_iclear),
	KOBJMETHOD(scc_ipend,	quicc_bfe_ipend),
	KOBJMETHOD(scc_probe,	quicc_bfe_probe),
	{ 0, 0 }
};

struct scc_class scc_quicc_class = {
	"QUICC class",
	quicc_methods,
	sizeof(struct scc_softc),
	.cl_channels = 4,
	.cl_class = SCC_CLASS_QUICC,
	.cl_modes = SCC_MODE_ASYNC | SCC_MODE_BISYNC | SCC_MODE_HDLC,
	.cl_range = 0,
};

static int
quicc_bfe_attach(struct scc_softc *sc, int reset)
{
	struct scc_bas *bas;

	bas = &sc->sc_bas;
	return (0);
}

static int
quicc_bfe_enabled(struct scc_softc *sc, struct scc_chan *ch)
{
	struct scc_bas *bas;
	int unit;
	uint16_t val0, val1;

	bas = &sc->sc_bas;
	unit = ch->ch_nr - 1;
	val0 = quicc_read2(bas, QUICC_REG_SCC_TODR(unit));
	quicc_write2(bas, QUICC_REG_SCC_TODR(unit), ~val0);
	val1 = quicc_read2(bas, QUICC_REG_SCC_TODR(unit));
	quicc_write2(bas, QUICC_REG_SCC_TODR(unit), val0);
	return (((val0 | val1) == 0x8000) ? 1 : 0);
}

static int
quicc_bfe_iclear(struct scc_softc *sc, struct scc_chan *ch)
{

	return (0);
}

static int
quicc_bfe_ipend(struct scc_softc *sc)
{
	struct scc_bas *bas;
	struct scc_chan *ch;
	int c, ipend;
	uint16_t scce;

	bas = &sc->sc_bas;
	ipend = 0;
	for (c = 0; c < 4; c++) {
		ch = &sc->sc_chan[c];
		if (!ch->ch_enabled)
			continue;
		ch->ch_ipend = 0;
		mtx_lock_spin(&sc->sc_hwmtx);
		scce = quicc_read2(bas, QUICC_REG_SCC_SCCE(c));
		quicc_write2(bas, QUICC_REG_SCC_SCCE(c), ~0);
		mtx_unlock_spin(&sc->sc_hwmtx);
		if (scce & 0x0001)
			ch->ch_ipend |= SER_INT_RXREADY;
		if (scce & 0x0002)
			ch->ch_ipend |= SER_INT_TXIDLE;
		if (scce & 0x0004)
			ch->ch_ipend |= SER_INT_OVERRUN;
		if (scce & 0x0020)
			ch->ch_ipend |= SER_INT_BREAK;
		/* XXX SIGNALS */
		ipend |= ch->ch_ipend;
	}
	return (ipend);
}

static int
quicc_bfe_probe(struct scc_softc *sc)
{
	struct scc_bas *bas;

	bas = &sc->sc_bas;
	return (0);
}
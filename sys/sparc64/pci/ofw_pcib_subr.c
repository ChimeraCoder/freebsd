
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

#include "opt_ofw_pci.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/rman.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_pci.h>
#include <dev/ofw/openfirm.h>

#include <machine/bus.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcib_private.h>

#include "pcib_if.h"

#include <sparc64/pci/ofw_pci.h>
#include <sparc64/pci/ofw_pcib_subr.h>

void
ofw_pcib_gen_setup(device_t bridge)
{
	struct ofw_pcib_gen_softc *sc;

	sc = device_get_softc(bridge);
	sc->ops_pcib_sc.dev = bridge;
	sc->ops_node = ofw_bus_get_node(bridge);
	KASSERT(sc->ops_node != 0,
	    ("ofw_pcib_gen_setup: no ofw pci parent bus!"));

	ofw_bus_setup_iinfo(sc->ops_node, &sc->ops_iinfo,
	    sizeof(ofw_pci_intr_t));
}

int
ofw_pcib_gen_route_interrupt(device_t bridge, device_t dev, int intpin)
{
	struct ofw_pcib_gen_softc *sc;
	struct ofw_bus_iinfo *ii;
	struct ofw_pci_register reg;
	ofw_pci_intr_t pintr, mintr;
	uint8_t maskbuf[sizeof(reg) + sizeof(pintr)];

	sc = device_get_softc(bridge);
	ii = &sc->ops_iinfo;
	if (ii->opi_imapsz > 0) {
		pintr = intpin;
		if (ofw_bus_lookup_imap(ofw_bus_get_node(dev), ii, &reg,
		    sizeof(reg), &pintr, sizeof(pintr), &mintr, sizeof(mintr),
		    NULL, maskbuf)) {
			/*
			 * If we've found a mapping, return it and don't map
			 * it again on higher levels - that causes problems
			 * in some cases, and never seems to be required.
			 */
			return (mintr);
		}
	} else if (intpin >= 1 && intpin <= 4) {
		/*
		 * When an interrupt map is missing, we need to do the
		 * standard PCI swizzle and continue mapping at the parent.
		 */
		return (pcib_route_interrupt(bridge, dev, intpin));
	}
	/* Try at the parent. */
	return (PCIB_ROUTE_INTERRUPT(device_get_parent(device_get_parent(
	    bridge)), bridge, intpin));
}

phandle_t
ofw_pcib_gen_get_node(device_t bridge, device_t dev)
{
	struct ofw_pcib_gen_softc *sc;

	sc = device_get_softc(bridge);
	return (sc->ops_node);
}
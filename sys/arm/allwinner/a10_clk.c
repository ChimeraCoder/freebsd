
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

/* Simple clock driver for Allwinner A10 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/rman.h>
#include <sys/timeet.h>
#include <sys/timetc.h>
#include <sys/watchdog.h>
#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/frame.h>
#include <machine/intr.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <machine/bus.h>
#include <machine/fdt.h>

#include "a10_clk.h"

struct a10_ccm_softc {
	struct resource		*res;
	bus_space_tag_t		bst;
	bus_space_handle_t	bsh;
};

static struct a10_ccm_softc *a10_ccm_sc = NULL;

#define ccm_read_4(sc, reg)		\
	bus_space_read_4((sc)->bst, (sc)->bsh, (reg))
#define ccm_write_4(sc, reg, val)	\
	bus_space_write_4((sc)->bst, (sc)->bsh, (reg), (val))

static int
a10_ccm_probe(device_t dev)
{
	if (ofw_bus_is_compatible(dev, "allwinner,sun4i-ccm")) {
		device_set_desc(dev, "Allwinner Clock Control Module");
		return(BUS_PROBE_DEFAULT);
	}

	return (ENXIO);
}

static int
a10_ccm_attach(device_t dev)
{
	struct a10_ccm_softc *sc = device_get_softc(dev);
	int rid = 0;

	if (a10_ccm_sc)
		return (ENXIO);

	sc->res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, RF_ACTIVE);
	if (!sc->res) {
		device_printf(dev, "could not allocate resource\n");
		return (ENXIO);
	}

	sc->bst = rman_get_bustag(sc->res);
	sc->bsh = rman_get_bushandle(sc->res);

	a10_ccm_sc = sc;

	return (0);
}

static device_method_t a10_ccm_methods[] = {
	DEVMETHOD(device_probe,		a10_ccm_probe),
	DEVMETHOD(device_attach,	a10_ccm_attach),
	{ 0, 0 }
};

static driver_t a10_ccm_driver = {
	"a10_ccm",
	a10_ccm_methods,
	sizeof(struct a10_ccm_softc),
};

static devclass_t a10_ccm_devclass;

DRIVER_MODULE(a10_ccm, simplebus, a10_ccm_driver, a10_ccm_devclass, 0, 0);

int
a10_clk_usb_activate(void)
{
	struct a10_ccm_softc *sc = a10_ccm_sc;
	uint32_t reg_value;

	if (sc == NULL)
		return ENXIO;

	/* Gating AHB clock for USB */
	reg_value = ccm_read_4(sc, CCM_AHB_GATING0);
	reg_value |= CCM_AHB_GATING_USB0; /* AHB clock gate usb0 */
	reg_value |= CCM_AHB_GATING_EHCI0; /* AHB clock gate ehci1 */
	reg_value |= CCM_AHB_GATING_EHCI1; /* AHB clock gate ehci1 */
	ccm_write_4(sc, CCM_AHB_GATING0, reg_value);

	/* Enable clock for USB */
	reg_value = ccm_read_4(sc, CCM_USB_CLK);
	reg_value |= CCM_USB_PHY; /* USBPHY */
	reg_value |= CCM_USB0_RESET; /* disable reset for USB0 */
	reg_value |= CCM_USB1_RESET; /* disable reset for USB1 */
	reg_value |= CCM_USB2_RESET; /* disable reset for USB2 */
	ccm_write_4(sc, CCM_USB_CLK, reg_value);

	return (0);
}

int
a10_clk_usb_deactivate(void)
{
	struct a10_ccm_softc *sc = a10_ccm_sc;
	uint32_t reg_value;

	if (sc == NULL)
		return ENXIO;

	/* Disable clock for USB */
	reg_value = ccm_read_4(sc, CCM_USB_CLK);
	reg_value &= ~CCM_USB_PHY; /* USBPHY */
	reg_value &= ~CCM_USB0_RESET; /* reset for USB0 */
	reg_value &= ~CCM_USB1_RESET; /* reset for USB1 */
	reg_value &= ~CCM_USB2_RESET; /* reset for USB2 */
	ccm_write_4(sc, CCM_USB_CLK, reg_value);

	/* Disable gating AHB clock for USB */
	reg_value = ccm_read_4(sc, CCM_AHB_GATING0);
	reg_value &= ~CCM_AHB_GATING_USB0; /* disable AHB clock gate usb0 */
	reg_value &= ~CCM_AHB_GATING_EHCI1; /* disable AHB clock gate ehci1 */
	ccm_write_4(sc, CCM_AHB_GATING0, reg_value);

	return (0);
}
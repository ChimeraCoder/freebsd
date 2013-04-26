
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
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#define INTC_REVISION		0x00
#define INTC_SYSCONFIG		0x10
#define INTC_SYSSTATUS		0x14
#define INTC_SIR_IRQ		0x40
#define INTC_CONTROL		0x48
#define INTC_THRESHOLD		0x68
#define INTC_MIR_CLEAR(x)	(0x88 + ((x) * 0x20))
#define INTC_MIR_SET(x)		(0x8C + ((x) * 0x20))
#define INTC_ISR_SET(x)		(0x90 + ((x) * 0x20))
#define INTC_ISR_CLEAR(x)	(0x94 + ((x) * 0x20))

struct ti_aintc_softc {
	device_t		sc_dev;
	struct resource *	aintc_res[3];
	bus_space_tag_t		aintc_bst;
	bus_space_handle_t	aintc_bsh;
	uint8_t			ver;
};

static struct resource_spec ti_aintc_spec[] = {
	{ SYS_RES_MEMORY,	0,	RF_ACTIVE },
	{ -1, 0 }
};


static struct ti_aintc_softc *ti_aintc_sc = NULL;

#define	aintc_read_4(reg)		\
    bus_space_read_4(ti_aintc_sc->aintc_bst, ti_aintc_sc->aintc_bsh, reg)
#define	aintc_write_4(reg, val)		\
    bus_space_write_4(ti_aintc_sc->aintc_bst, ti_aintc_sc->aintc_bsh, reg, val)


static int
ti_aintc_probe(device_t dev)
{
	if (!ofw_bus_is_compatible(dev, "ti,aintc"))
		return (ENXIO);
	device_set_desc(dev, "TI AINTC Interrupt Controller");
	return (BUS_PROBE_DEFAULT);
}

static int
ti_aintc_attach(device_t dev)
{
	struct		ti_aintc_softc *sc = device_get_softc(dev);
	uint32_t x;

	sc->sc_dev = dev;

	if (ti_aintc_sc)
		return (ENXIO);

	if (bus_alloc_resources(dev, ti_aintc_spec, sc->aintc_res)) {
		device_printf(dev, "could not allocate resources\n");
		return (ENXIO);
	}

	sc->aintc_bst = rman_get_bustag(sc->aintc_res[0]);
	sc->aintc_bsh = rman_get_bushandle(sc->aintc_res[0]);

	ti_aintc_sc = sc;

	x = aintc_read_4(INTC_REVISION);
	device_printf(dev, "Revision %u.%u\n",(x >> 4) & 0xF, x & 0xF);

	/* SoftReset */
	aintc_write_4(INTC_SYSCONFIG, 2);

	/* Wait for reset to complete */
	while(!(aintc_read_4(INTC_SYSSTATUS) & 1));

	/*Set Priority Threshold */
	aintc_write_4(INTC_THRESHOLD, 0xFF);

	return (0);
}

static device_method_t ti_aintc_methods[] = {
	DEVMETHOD(device_probe,		ti_aintc_probe),
	DEVMETHOD(device_attach,	ti_aintc_attach),
	{ 0, 0 }
};

static driver_t ti_aintc_driver = {
	"aintc",
	ti_aintc_methods,
	sizeof(struct ti_aintc_softc),
};

static devclass_t ti_aintc_devclass;

DRIVER_MODULE(aintc, simplebus, ti_aintc_driver, ti_aintc_devclass, 0, 0);

int
arm_get_next_irq(int last_irq)
{
	uint32_t active_irq;

	if (last_irq != -1) {
		aintc_write_4(INTC_ISR_CLEAR(last_irq >> 5),
			1UL << (last_irq & 0x1F));
		aintc_write_4(INTC_CONTROL,1);
	}

	/* Get the next active interrupt */
	active_irq = aintc_read_4(INTC_SIR_IRQ);

	/* Check for spurious interrupt */
	if ((active_irq & 0xffffff80)) {
		device_printf(ti_aintc_sc->sc_dev,
			"Spurious interrupt detected (0x%08x)\n", active_irq);
		aintc_write_4(INTC_SIR_IRQ, 0);
		return -1;
	}

	if (active_irq != last_irq)
		return active_irq;
	else
		return -1;
}

void
arm_mask_irq(uintptr_t nb)
{
	aintc_write_4(INTC_MIR_SET(nb >> 5), (1UL << (nb & 0x1F)));
}

void
arm_unmask_irq(uintptr_t nb)
{
	aintc_write_4(INTC_MIR_CLEAR(nb >> 5), (1UL << (nb & 0x1F)));
}
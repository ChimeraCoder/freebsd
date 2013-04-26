
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

#define	INTC_PENDING_BASIC	0x00
#define	INTC_PENDING_BANK1	0x04
#define	INTC_PENDING_BANK2	0x08
#define	INTC_FIQ_CONTROL	0x0C
#define	INTC_ENABLE_BANK1	0x10
#define	INTC_ENABLE_BANK2	0x14
#define	INTC_ENABLE_BASIC	0x18
#define	INTC_DISABLE_BANK1	0x1C
#define	INTC_DISABLE_BANK2	0x20
#define	INTC_DISABLE_BASIC	0x24

#define	BANK1_START	8
#define	BANK1_END	(BANK1_START + 32 - 1)
#define	BANK2_START	(BANK1_START + 32)
#define	BANK2_END	(BANK2_START + 32 - 1)

#define	IS_IRQ_BASIC(n)	(((n) >= 0) && ((n) < BANK1_START))
#define	IS_IRQ_BANK1(n)	(((n) >= BANK1_START) && ((n) <= BANK1_END))
#define	IS_IRQ_BANK2(n)	(((n) >= BANK2_START) && ((n) <= BANK2_END))
#define	IRQ_BANK1(n)	((n) - BANK1_START)
#define	IRQ_BANK2(n)	((n) - BANK2_START)

#ifdef  DEBUG
#define dprintf(fmt, args...) printf(fmt, ##args)
#else
#define dprintf(fmt, args...)
#endif

struct bcm_intc_softc {
	device_t		sc_dev;
	struct resource *	intc_res;
	bus_space_tag_t		intc_bst;
	bus_space_handle_t	intc_bsh;
};

static struct bcm_intc_softc *bcm_intc_sc = NULL;

#define	intc_read_4(reg)		\
    bus_space_read_4(bcm_intc_sc->intc_bst, bcm_intc_sc->intc_bsh, reg)
#define	intc_write_4(reg, val)		\
    bus_space_write_4(bcm_intc_sc->intc_bst, bcm_intc_sc->intc_bsh, reg, val)

static int
bcm_intc_probe(device_t dev)
{
	if (!ofw_bus_is_compatible(dev, "broadcom,bcm2835-armctrl-ic"))
		return (ENXIO);
	device_set_desc(dev, "BCM2835 Interrupt Controller");
	return (BUS_PROBE_DEFAULT);
}

static int
bcm_intc_attach(device_t dev)
{
	struct		bcm_intc_softc *sc = device_get_softc(dev);
	int		rid = 0;

	sc->sc_dev = dev;

	if (bcm_intc_sc)
		return (ENXIO);

	sc->intc_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, RF_ACTIVE);
	if (sc->intc_res == NULL) {
		device_printf(dev, "could not allocate memory resource\n");
		return (ENXIO);
	}

	sc->intc_bst = rman_get_bustag(sc->intc_res);
	sc->intc_bsh = rman_get_bushandle(sc->intc_res);

	bcm_intc_sc = sc;

	return (0);
}

static device_method_t bcm_intc_methods[] = {
	DEVMETHOD(device_probe,		bcm_intc_probe),
	DEVMETHOD(device_attach,	bcm_intc_attach),
	{ 0, 0 }
};

static driver_t bcm_intc_driver = {
	"intc",
	bcm_intc_methods,
	sizeof(struct bcm_intc_softc),
};

static devclass_t bcm_intc_devclass;

DRIVER_MODULE(intc, simplebus, bcm_intc_driver, bcm_intc_devclass, 0, 0);

int
arm_get_next_irq(int last_irq)
{
	uint32_t pending;
	int32_t irq = last_irq + 1;

	/* Sanity check */
	if (irq < 0)
		irq = 0;
	
	/* TODO: should we mask last_irq? */
	pending = intc_read_4(INTC_PENDING_BASIC);
	while (irq < BANK1_START) {
		if (pending & (1 << irq))
			return irq;
		irq++;
	}

	pending = intc_read_4(INTC_PENDING_BANK1);
	while (irq < BANK2_START) {
		if (pending & (1 << IRQ_BANK1(irq)))
			return irq;
		irq++;
	}

	pending = intc_read_4(INTC_PENDING_BANK2);
	while (irq <= BANK2_END) {
		if (pending & (1 << IRQ_BANK2(irq)))
			return irq;
		irq++;
	}

	return (-1);
}

void
arm_mask_irq(uintptr_t nb)
{
	dprintf("%s: %d\n", __func__, nb);

	if (IS_IRQ_BASIC(nb))
		intc_write_4(INTC_DISABLE_BASIC, (1 << nb));
	else if (IS_IRQ_BANK1(nb))
		intc_write_4(INTC_DISABLE_BANK1, (1 << IRQ_BANK1(nb)));
	else if (IS_IRQ_BANK2(nb))
		intc_write_4(INTC_DISABLE_BANK2, (1 << IRQ_BANK2(nb)));
	else
		printf("arm_mask_irq: Invalid IRQ number: %d\n", nb);
}

void
arm_unmask_irq(uintptr_t nb)
{
	dprintf("%s: %d\n", __func__, nb);

	if (IS_IRQ_BASIC(nb))
		intc_write_4(INTC_ENABLE_BASIC, (1 << nb));
	else if (IS_IRQ_BANK1(nb))
		intc_write_4(INTC_ENABLE_BANK1, (1 << IRQ_BANK1(nb)));
	else if (IS_IRQ_BANK2(nb))
		intc_write_4(INTC_ENABLE_BANK2, (1 << IRQ_BANK2(nb)));
	else
		printf("arm_mask_irq: Invalid IRQ number: %d\n", nb);
}
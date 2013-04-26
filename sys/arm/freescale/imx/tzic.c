
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
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/cpuset.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <arm/freescale/imx/imx51_tzicreg.h>

struct tzic_softc {
	struct resource *	tzic_res[3];
	bus_space_tag_t		tzic_bst;
	bus_space_handle_t	tzic_bsh;
	uint8_t			ver;
};

static struct resource_spec tzic_spec[] = {
	{ SYS_RES_MEMORY,	0,	RF_ACTIVE },
	{ -1, 0 }
};

static struct tzic_softc *tzic_sc = NULL;

#define	tzic_read_4(reg)		\
    bus_space_read_4(tzic_sc->tzic_bst, tzic_sc->tzic_bsh, reg)
#define	tzic_write_4(reg, val)		\
    bus_space_write_4(tzic_sc->tzic_bst, tzic_sc->tzic_bsh, reg, val)

static void tzic_post_filter(void *);

static int
tzic_probe(device_t dev)
{
	if (ofw_bus_is_compatible(dev, "fsl,tzic")) {
		device_set_desc(dev, "TrustZone Interrupt Controller");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
tzic_attach(device_t dev)
{
	struct		tzic_softc *sc = device_get_softc(dev);
	int		i;
	uint32_t	reg;

	if (tzic_sc)
		return (ENXIO);

	if (bus_alloc_resources(dev, tzic_spec, sc->tzic_res)) {
		device_printf(dev, "could not allocate resources\n");
		return (ENXIO);
	}

	arm_post_filter = tzic_post_filter;

	/* Distributor Interface */
	sc->tzic_bst = rman_get_bustag(sc->tzic_res[0]);
	sc->tzic_bsh = rman_get_bushandle(sc->tzic_res[0]);

	tzic_sc = sc;

	reg = tzic_read_4(TZIC_INTCNTL);
	tzic_write_4(TZIC_INTCNTL, INTCNTL_NSEN_MASK|INTCNTL_NSEN|INTCNTL_EN);
	reg = tzic_read_4(TZIC_INTCNTL);
	tzic_write_4(TZIC_PRIOMASK, 0x1f);
	reg = tzic_read_4(TZIC_PRIOMASK);

	tzic_write_4(TZIC_SYNCCTRL, 0x02);
	reg = tzic_read_4(TZIC_SYNCCTRL);

	/* route all interrupts to IRQ.  secure interrupts are for FIQ */
	for (i = 0; i < 4; i++)
		tzic_write_4(TZIC_INTSEC(i), 0xffffffff);

	/* disable all interrupts */
	for (i = 0; i < 4; i++)
		tzic_write_4(TZIC_ENCLEAR(i), 0xffffffff);

	return (0);
}

static device_method_t tzic_methods[] = {
	DEVMETHOD(device_probe,		tzic_probe),
	DEVMETHOD(device_attach,	tzic_attach),
	{ 0, 0 }
};

static driver_t tzic_driver = {
	"tzic",
	tzic_methods,
	sizeof(struct tzic_softc),
};

static devclass_t tzic_devclass;

/*
 * Memory space of controller located outside of device range, so let him to
 * attach not only to simplebus, but fdtbus also.
 */
EARLY_DRIVER_MODULE(tzic, fdtbus, tzic_driver, tzic_devclass, 0, 0,
    BUS_PASS_INTERRUPT);
EARLY_DRIVER_MODULE(tzic, simplebus, tzic_driver, tzic_devclass, 0, 0,
    BUS_PASS_INTERRUPT);

static void
tzic_post_filter(void *arg)
{

}

int
arm_get_next_irq(int last_irq)
{
	uint32_t pending;
	int i, b;

	for (i = 0; i < 4; i++) {
		pending = tzic_read_4(TZIC_PND(i));
		for (b = 0; b < 32; b++)
			if (pending & (1 << b)) {
				return (i * 32 + b);
			}
	}

	return (-1);
}

void
arm_mask_irq(uintptr_t nb)
{

	tzic_write_4(TZIC_ENCLEAR(nb / 32), (1UL <<(nb % 32)));
}

void
arm_unmask_irq(uintptr_t nb)
{

	tzic_write_4(TZIC_ENSET(nb / 32), (1UL <<(nb % 32)));
}

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
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>
#include <isa/isavar.h>

#define UPD7210_HW_DRIVER
#include <dev/ieee488/upd7210.h>

struct pcii_softc {
	int foo;
	struct resource	*res[11];
	void *intr_handler;
	struct upd7210	upd7210;
};

static devclass_t pcii_devclass;

static int	pcii_probe(device_t dev);
static int	pcii_attach(device_t dev);

static device_method_t pcii_methods[] = {
	DEVMETHOD(device_probe,		pcii_probe),
	DEVMETHOD(device_attach,	pcii_attach),
	DEVMETHOD(device_suspend,	bus_generic_suspend),
	DEVMETHOD(device_resume,	bus_generic_resume),

	{ 0, 0 }
};

static struct resource_spec pcii_res_spec[] = {
	{ SYS_RES_IRQ,		0, RF_ACTIVE | RF_SHAREABLE},
	{ SYS_RES_DRQ,		0, RF_ACTIVE | RF_SHAREABLE | RF_OPTIONAL},
	{ SYS_RES_IOPORT,	0, RF_ACTIVE},
	{ SYS_RES_IOPORT,	1, RF_ACTIVE},
	{ SYS_RES_IOPORT,	2, RF_ACTIVE},
	{ SYS_RES_IOPORT,	3, RF_ACTIVE},
	{ SYS_RES_IOPORT,	4, RF_ACTIVE},
	{ SYS_RES_IOPORT,	5, RF_ACTIVE},
	{ SYS_RES_IOPORT,	6, RF_ACTIVE},
	{ SYS_RES_IOPORT,	7, RF_ACTIVE},
	{ SYS_RES_IOPORT,	8, RF_ACTIVE | RF_SHAREABLE},
	{ -1, 0, 0 }
};

static driver_t pcii_driver = {
	"pcii",
	pcii_methods,
	sizeof(struct pcii_softc),
};

static int
pcii_probe(device_t dev)
{
	int rid, i, j;
	u_long start, count, addr;
	int error = 0;
	struct pcii_softc *sc;

	device_set_desc(dev, "PCII IEEE-4888 controller");
	sc = device_get_softc(dev);

	rid = 0;
	if (bus_get_resource(dev, SYS_RES_IOPORT, rid, &start, &count) != 0)
		return ENXIO;
	/*
	 * The PCIIA decodes a fixed pattern of 0x2e1 for the lower 10
	 * address bits A0 ... A9.  Bits A10 through A12 are used by
	 * the µPD7210 register select lines.  This makes the
	 * individual 7210 register being 0x400 bytes apart in the ISA
	 * bus address space.  Address bits A13 and A14 are compared
	 * to a DIP switch setting on the card, allowing for up to 4
	 * different cards being installed (at base addresses 0x2e1,
	 * 0x22e1, 0x42e1, and 0x62e1, respectively).  A15 has been
	 * used to select an optional on-board time-of-day clock chip
	 * (MM58167A) on the original PCIIA rather than the µPD7210
	 * (which is not implemented on later boards).  The
	 * documentation states the respective addresses for that chip
	 * should be handled as reserved addresses, which we don't do
	 * (right now).  Finally, the IO addresses 0x2f0 ... 0x2f7 for
	 * a "special interrupt handling feature" (re-enable
	 * interrupts so the IRQ can be shared).
	 *
	 * Usually, the user will only set the base address in the
	 * device hints, so we handle the rest here.
	 *
	 * (Source: GPIB-PCIIA Technical Reference Manual, September
	 * 1989 Edition, National Instruments.)
	 */
	if ((start & 0x3ff) != 0x2e1) {
		if (bootverbose)
			printf("pcii_probe: PCIIA base address 0x%lx not "
			       "0x2e1/0x22e1/0x42e1/0x62e1\n",
			       start);
		return (ENXIO);
	}

	for (rid = 0, addr = start; rid < 8; rid++, addr += 0x400) {
		if (bus_set_resource(dev, SYS_RES_IOPORT, rid, addr, 1) != 0) {
			printf("pcii_probe: could not set IO port 0x%lx\n",
			       addr);
			return (ENXIO);
		}
	}
	if (bus_get_resource(dev, SYS_RES_IRQ, 0, &start, &count) != 0) {
		printf("pcii_probe: cannot obtain IRQ level\n");
		return ENXIO;
	}
	if (start > 7) {
		printf("pcii_probe: IRQ level %lu too high\n", start);
		return ENXIO;
	}

	if (bus_set_resource(dev, SYS_RES_IOPORT, 8, 0x2f0 + start, 1) != 0) {
		printf("pcii_probe: could not set IO port 0x%3lx\n",
		       0x2f0 + start);
		return (ENXIO);
	}

	error = bus_alloc_resources(dev, pcii_res_spec, sc->res);
	if (error) {
		printf("pcii_probe: Could not allocate resources\n");
		return (error);
	}
	error = ENXIO;
	/*
	 * Perform some basic tests on the µPD7210 registers.  At
	 * least *some* register must read different from 0x00 or
	 * 0xff.
	 */
	for (i = 0; i < 8; i++) {
		j = bus_read_1(sc->res[2 + i], 0);
		if (j != 0x00 && j != 0xff)
			error = 0;
	}
	/* SPSR/SPMR read/write test */
	if (!error) {
		bus_write_1(sc->res[2 + 3], 0, 0x55);
		if (bus_read_1(sc->res[2 + 3], 0) != 0x55)
			error = ENXIO;
	}
	if (!error) {
		bus_write_1(sc->res[2 + 3], 0, 0xaa);
		if (bus_read_1(sc->res[2 + 3], 0) != 0xaa)
			error = ENXIO;
	}
	if (error)
		printf("pcii_probe: probe failure\n");

	bus_release_resources(dev, pcii_res_spec, sc->res);
	return (error);
}

static int
pcii_attach(device_t dev)
{
	struct pcii_softc *sc;
	u_long		start, count;
	int		unit;
	int		rid;
	int		error = 0;

	unit = device_get_unit(dev);
	sc = device_get_softc(dev);
	memset(sc, 0, sizeof *sc);

	device_set_desc(dev, "PCII IEEE-4888 controller");

	if (bus_get_resource(dev, SYS_RES_IRQ, 0, &start, &count) != 0) {
		printf("pcii_attach: cannot obtain IRQ number\n");
		return ENXIO;
	}

	error = bus_alloc_resources(dev, pcii_res_spec, sc->res);
	if (error)
		return (error);

	error = bus_setup_intr(dev, sc->res[0],
	    INTR_TYPE_MISC | INTR_MPSAFE, NULL,
	    upd7210intr, &sc->upd7210, &sc->intr_handler);
	if (error) {
		bus_release_resources(dev, pcii_res_spec, sc->res);
		return (error);
	}

	for (rid = 0; rid < 8; rid++) {
		sc->upd7210.reg_res[rid] = sc->res[2 + rid];
		sc->upd7210.reg_offset[rid] = 0;
	}
	sc->upd7210.irq_clear_res = sc->res[10];
	sc->upd7210.use_fifo = 0;

	if (sc->res[1] == NULL)
		sc->upd7210.dmachan = -1;
	else
		sc->upd7210.dmachan = rman_get_start(sc->res[1]);

	upd7210attach(&sc->upd7210);
	device_printf(dev, "attached gpib%d\n", sc->upd7210.unit);

	return (0);
}

DRIVER_MODULE(pcii, isa, pcii_driver, pcii_devclass, 0, 0);
DRIVER_MODULE(pcii, acpi, pcii_driver, pcii_devclass, 0, 0);
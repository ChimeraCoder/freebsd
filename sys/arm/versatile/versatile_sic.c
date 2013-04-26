
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

#ifdef  DEBUG
#define dprintf(fmt, args...) printf(fmt, ##args)
#else
#define dprintf(fmt, args...)
#endif

#define	SIC_STATUS	0x00
#define	SIC_RAWSTAT	0x04
#define	SIC_ENABLE	0x08
#define	SIC_ENSET	0x08
#define	SIC_ENCLR	0x0C
#define	SIC_SOFTINTSET	0x10
#define	SIC_SOFTINTCLR	0x14
#define	SIC_PICENABLE	0x20
#define	SIC_PICENSET	0x20
#define	SIC_PICENCLR	0x24

struct versatile_sic_softc {
	device_t		sc_dev;
	struct resource *	mem_res;
};

#define	sic_read_4(sc, reg)			\
    bus_read_4(sc->mem_res, (reg))
#define	sic_write_4(sc, reg, val)		\
    bus_write_4(sc->mem_res, (reg), (val))

static int
versatile_sic_probe(device_t dev)
{
	if (!ofw_bus_is_compatible(dev, "arm,versatile-sic"))
		return (ENXIO);
	device_set_desc(dev, "ARM Versatile SIC");
	return (BUS_PROBE_DEFAULT);
}

static int
versatile_sic_attach(device_t dev)
{
	struct		versatile_sic_softc *sc = device_get_softc(dev);
	uint32_t	pass_irqs;
	int		rid;

	sc->sc_dev = dev;

	/* Request memory resources */
	rid = 0;
	sc->mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid,
	    RF_ACTIVE);
	if (sc->mem_res == NULL) {
		device_printf(dev, "Error: could not allocate memory resources\n");
		return (ENXIO);
	}

	/* Disable all interrupts on SIC */
	sic_write_4(sc, SIC_ENCLR, 0xffffffff);

	/* 
	 * XXX: Enable IRQ3 for KMI
	 * Should be replaced by proper interrupts cascading
	 */
	sic_write_4(sc, SIC_ENSET, (1 << 3));

	/*
	 * Let PCI and Ethernet interrupts pass through
	 * IRQ25, IRQ27..IRQ31
	 */
	pass_irqs = (7 << 27) | (1 << 25);
	sic_write_4(sc, SIC_PICENSET, pass_irqs);

	return (0);
}

static device_method_t versatile_sic_methods[] = {
	DEVMETHOD(device_probe,		versatile_sic_probe),
	DEVMETHOD(device_attach,	versatile_sic_attach),
	{ 0, 0 }
};

static driver_t versatile_sic_driver = {
	"sic",
	versatile_sic_methods,
	sizeof(struct versatile_sic_softc),
};

static devclass_t versatile_sic_devclass;

DRIVER_MODULE(sic, simplebus, versatile_sic_driver, versatile_sic_devclass, 0, 0);
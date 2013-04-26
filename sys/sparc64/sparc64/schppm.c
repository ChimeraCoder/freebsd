
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
#include <sys/module.h>
#include <sys/resource.h>
#include <sys/rman.h>

#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>
#include <machine/resource.h>

#define	SCHPPM_NREG	1

#define	SCHPPM_ESTAR	0

#define	SCHPPM_ESTAR_CTRL		0x00
#define	SCHPPM_ESTAR_CTRL_1		0x00000001
#define	SCHPPM_ESTAR_CTRL_2		0x00000002
#define	SCHPPM_ESTAR_CTRL_32		0x00000020
#define	SCHPPM_ESTAR_CTRL_MASK						\
	(SCHPPM_ESTAR_CTRL_1 | SCHPPM_ESTAR_CTRL_2 | SCHPPM_ESTAR_CTRL_32)

static struct resource_spec schppm_res_spec[] = {
	{ SYS_RES_MEMORY, SCHPPM_ESTAR, RF_ACTIVE },
	{ -1, 0 }
};

struct schppm_softc {
	struct resource		*sc_res[SCHPPM_NREG];
};

#define	SCHPPM_READ(sc, reg, off)					\
	bus_read_8((sc)->sc_res[(reg)], (off))
#define	SCHPPM_WRITE(sc, reg, off, val)					\
	bus_write_8((sc)->sc_res[(reg)], (off), (val))

static device_probe_t schppm_probe;
static device_attach_t schppm_attach;

static device_method_t schppm_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		schppm_probe),
	DEVMETHOD(device_attach,	schppm_attach),

	DEVMETHOD_END
};

static devclass_t schppm_devclass;

DEFINE_CLASS_0(schppm, schppm_driver, schppm_methods,
    sizeof(struct schppm_softc));
DRIVER_MODULE(schppm, nexus, schppm_driver, schppm_devclass, 0, 0);

static int
schppm_probe(device_t dev)
{
	const char* compat;

	compat = ofw_bus_get_compat(dev);
	if (compat != NULL && strcmp(ofw_bus_get_name(dev), "ppm") == 0 &&
	    strcmp(compat, "gp2-ppm") == 0) {
		device_set_desc(dev, "Schizo power management");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
schppm_attach(device_t dev)
{
	struct schppm_softc *sc;

	sc = device_get_softc(dev);
	if (bus_alloc_resources(dev, schppm_res_spec, sc->sc_res)) {
		device_printf(dev, "failed to allocate resources\n");
		bus_release_resources(dev, schppm_res_spec, sc->sc_res);
		return (ENXIO);
	}

	if (bootverbose) {
		device_printf(dev, "running at ");
		switch (SCHPPM_READ(sc, SCHPPM_ESTAR, SCHPPM_ESTAR_CTRL) &
		    SCHPPM_ESTAR_CTRL_MASK) {
		case SCHPPM_ESTAR_CTRL_1:
			printf("full");
			break;
		case SCHPPM_ESTAR_CTRL_2:
			printf("half");
			break;
		case SCHPPM_ESTAR_CTRL_32:
			printf("1/32");
			break;
		default:
			printf("unknown");
			break;
		}
		printf(" speed\n");
	}

	return (0);
}
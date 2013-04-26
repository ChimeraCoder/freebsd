
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

#if 1
#include <sparc64/pci/ofw_pci.h>
#endif

#define	JBUSPPM_NREG	2

#define	JBUSPPM_DEVID	0
#define	JBUSPPM_ESTAR	1

#define	JBUSPPM_DEVID_JID		0x00
#define	JBUSPPM_DEVID_JID_MASTER	0x00040000

#define	JBUSPPM_ESTAR_CTRL		0x00
#define	JBUSPPM_ESTAR_CTRL_1		0x00000001
#define	JBUSPPM_ESTAR_CTRL_2		0x00000002
#define	JBUSPPM_ESTAR_CTRL_32		0x00000020
#define	JBUSPPM_ESTAR_CTRL_MASK						\
	(JBUSPPM_ESTAR_CTRL_1 | JBUSPPM_ESTAR_CTRL_2 | JBUSPPM_ESTAR_CTRL_32)
#define	JBUSPPM_ESTAR_JCHNG		0x08
#define	JBUSPPM_ESTAR_JCHNG_DELAY_MASK	0x00000007
#define	JBUSPPM_ESTAR_JCHNG_START	0x00000010
#define	JBUSPPM_ESTAR_JCHNG_OCCURED	0x00000018

struct jbusppm_softc {
	struct resource		*sc_res[JBUSPPM_NREG];
	bus_space_tag_t		sc_bt[JBUSPPM_NREG];
	bus_space_handle_t	sc_bh[JBUSPPM_NREG];
};

#define	JBUSPPM_READ(sc, reg, off)					\
	bus_space_read_8((sc)->sc_bt[(reg)], (sc)->sc_bh[(reg)], (off))
#define	JBUSPPM_WRITE(sc, reg, off, val)				\
	bus_space_write_8((sc)->sc_bt[(reg)], (sc)->sc_bh[(reg)], (off), (val))

static device_probe_t jbusppm_probe;
static device_attach_t jbusppm_attach;

static device_method_t jbusppm_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		jbusppm_probe),
	DEVMETHOD(device_attach,	jbusppm_attach),

	DEVMETHOD_END
};

static devclass_t jbusppm_devclass;

DEFINE_CLASS_0(jbusppm, jbusppm_driver, jbusppm_methods,
    sizeof(struct jbusppm_softc));
DRIVER_MODULE(jbusppm, nexus, jbusppm_driver, jbusppm_devclass, 0, 0);

static int
jbusppm_probe(device_t dev)
{
	const char* compat;

	compat = ofw_bus_get_compat(dev);
	if (compat != NULL && strcmp(ofw_bus_get_name(dev), "ppm") == 0 &&
	    strcmp(compat, "jbus-ppm") == 0) {
		device_set_desc(dev, "JBus power management");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
jbusppm_attach(device_t dev)
{
	struct jbusppm_softc *sc;
	int i, rid;
#if 1
	device_t *children, tomatillo;
	u_long tcount, tstart, jcount, jstart;
	int j, nchildren;
#endif

	sc = device_get_softc(dev);
	for (i = JBUSPPM_DEVID; i <= JBUSPPM_ESTAR; i++) {
		rid = i;
		/*
		 * The JBUSPPM_ESTAR resources is shared with that of the
		 * Tomatillo bus A controller configuration register bank.
		 */
#if 0
		sc->sc_res[i] = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
		    &rid, (i == JBUSPPM_ESTAR ? RF_SHAREABLE : 0) | RF_ACTIVE);
		if (sc->sc_res[i] == NULL) {
			device_printf(dev,
			    "could not allocate resource %d\n", i);
			goto fail;
		}
		sc->sc_bt[i] = rman_get_bustag(sc->sc_res[i]);
		sc->sc_bh[i] = rman_get_bushandle(sc->sc_res[i]);
#else
		/*
		 * Workaround for the fact that rman(9) only allows to
		 * share resources of the same size.
		 */
		if (i == JBUSPPM_ESTAR) {
			if (bus_get_resource(dev, SYS_RES_MEMORY, i, &jstart,
			    &jcount) != 0) {
				device_printf(dev,
				    "could not determine Estar resource\n");
				goto fail;
			}
			if (device_get_children(device_get_parent(dev),
			    &children, &nchildren) != 0) {
				device_printf(dev, "could not get children\n");
				goto fail;
			}
			tomatillo = NULL;
			for (j = 0; j < nchildren; j++) {
				if (ofw_bus_get_type(children[j]) != NULL &&
				    strcmp(ofw_bus_get_type(children[j]),
				    OFW_TYPE_PCI) == 0 &&
				    ofw_bus_get_compat(children[j]) != NULL &&
				    strcmp(ofw_bus_get_compat(children[j]),
				    "pci108e,a801") == 0 &&
				    ((bus_get_resource_start(children[j],
				    SYS_RES_MEMORY, 0) >> 20) & 1) == 0) {
					tomatillo = children[j];
					break;
				}
			}
			free(children, M_TEMP);
			if (tomatillo == NULL) {
				device_printf(dev,
				    "could not find Tomatillo\n");
				goto fail;
			}
			if (bus_get_resource(tomatillo, SYS_RES_MEMORY, 1,
			    &tstart, &tcount) != 0) {
				device_printf(dev,
				    "could not determine Tomatillo "
				    "resource\n");
				goto fail;
			}
			sc->sc_res[i] = bus_alloc_resource(dev, SYS_RES_MEMORY,
			    &rid, tstart, tstart + tcount - 1, tcount,
			    RF_SHAREABLE | RF_ACTIVE);
		} else
			sc->sc_res[i] = bus_alloc_resource_any(dev,
			    SYS_RES_MEMORY, &rid, RF_ACTIVE);
		if (sc->sc_res[i] == NULL) {
			device_printf(dev,
			    "could not allocate resource %d\n", i);
			goto fail;
		}
		sc->sc_bt[i] = rman_get_bustag(sc->sc_res[i]);
		sc->sc_bh[i] = rman_get_bushandle(sc->sc_res[i]);
		if (i == JBUSPPM_ESTAR)
			bus_space_subregion(sc->sc_bt[i], sc->sc_bh[i],
			    jstart - tstart, jcount, &sc->sc_bh[i]);
#endif
	}

	if (bootverbose) {
		if ((JBUSPPM_READ(sc, JBUSPPM_DEVID, JBUSPPM_DEVID_JID) &
		    JBUSPPM_DEVID_JID_MASTER) != 0)
			device_printf(dev, "master I/O bridge\n");
		device_printf(dev, "running at ");
		switch (JBUSPPM_READ(sc, JBUSPPM_ESTAR, JBUSPPM_ESTAR_CTRL) &
		    JBUSPPM_ESTAR_CTRL_MASK) {
		case JBUSPPM_ESTAR_CTRL_1:
			printf("full");
			break;
		case JBUSPPM_ESTAR_CTRL_2:
			printf("half");
			break;
		case JBUSPPM_ESTAR_CTRL_32:
			printf("1/32");
			break;
		default:
			printf("unknown");
			break;
		}
		printf(" speed\n");
	}

	return (0);

 fail:
	for (i = JBUSPPM_DEVID; i <= JBUSPPM_ESTAR && sc->sc_res[i] != NULL;
	    i++)
		bus_release_resource(dev, SYS_RES_MEMORY,
		    rman_get_rid(sc->sc_res[i]), sc->sc_res[i]);
	return (ENXIO);
}
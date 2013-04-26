
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
#include <sys/bus.h>
#include <sys/systm.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/rman.h>
#include <sys/sysctl.h>

#include <machine/bus.h>

#include <dev/ofw/openfirm.h>

#define NVIDIA_BRIGHT_MIN     (0x0ec)
#define NVIDIA_BRIGHT_MAX     (0x538)
#define NVIDIA_BRIGHT_SCALE   ((NVIDIA_BRIGHT_MAX - NVIDIA_BRIGHT_MIN)/100)
/* nVidia's MMIO registers are at PCI BAR[0] */
#define NVIDIA_MMIO_PMC       (0x0)
#define  NVIDIA_PMC_OFF         (NVIDIA_MMIO_PMC + 0x10f0)
#define   NVIDIA_PMC_BL_SHIFT    (16)
#define   NVIDIA_PMC_BL_EN       (1 << 31)


struct nvbl_softc {
	device_t	 dev;
	struct resource *sc_memr;
};

static void nvbl_identify(driver_t *driver, device_t parent);
static int nvbl_probe(device_t dev);
static int nvbl_attach(device_t dev);
static int nvbl_setlevel(struct nvbl_softc *sc, int newlevel);
static int nvbl_getlevel(struct nvbl_softc *sc);
static int nvbl_sysctl(SYSCTL_HANDLER_ARGS);

static device_method_t nvbl_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify, nvbl_identify),
	DEVMETHOD(device_probe, nvbl_probe),
	DEVMETHOD(device_attach, nvbl_attach),
	{0, 0},
};

static driver_t	nvbl_driver = {
	"backlight",
	nvbl_methods,
	sizeof(struct nvbl_softc)
};

static devclass_t nvbl_devclass;

DRIVER_MODULE(nvbl, vgapci, nvbl_driver, nvbl_devclass, 0, 0);

static void
nvbl_identify(driver_t *driver, device_t parent)
{
	if (device_find_child(parent, "backlight", -1) == NULL)
		device_add_child(parent, "backlight", -1);
}

static int
nvbl_probe(device_t dev)
{
	char		control[8];
	phandle_t	handle;

	handle = OF_finddevice("mac-io/backlight");

	if (handle == -1)
		return (ENXIO);

	if (OF_getprop(handle, "backlight-control", &control, sizeof(control)) < 0)
		return (ENXIO);

	if (strcmp(control, "mnca") != 0)
		return (ENXIO);

	device_set_desc(dev, "PowerBook backlight for nVidia graphics");

	return (0);
}

static int
nvbl_attach(device_t dev)
{
	struct nvbl_softc	*sc;
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid *tree;
	int			 rid;

	sc = device_get_softc(dev);

	rid = 0x10;	/* BAR[0], for the MMIO register */
	sc->sc_memr = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid,
			RF_ACTIVE | RF_SHAREABLE);
	if (sc->sc_memr == NULL) {
		device_printf(dev, "Could not alloc mem resource!\n");
		return (ENXIO);
	}

	/* Turn on big-endian mode */
	if (!(bus_read_stream_4(sc->sc_memr, NVIDIA_MMIO_PMC + 4) & 0x01000001)) {
		bus_write_stream_4(sc->sc_memr, NVIDIA_MMIO_PMC + 4, 0x01000001);
		mb();
	}

	ctx = device_get_sysctl_ctx(dev);
	tree = device_get_sysctl_tree(dev);

	SYSCTL_ADD_PROC(ctx, SYSCTL_CHILDREN(tree), OID_AUTO,
			"level", CTLTYPE_INT | CTLFLAG_RW, sc, 0,
			nvbl_sysctl, "I", "Backlight level (0-100)");

	return (0);
}

static int
nvbl_setlevel(struct nvbl_softc *sc, int newlevel)
{
	uint32_t pmc_reg;

	if (newlevel > 100)
		newlevel = 100;

	if (newlevel < 0)
		newlevel = 0;

	if (newlevel > 0)
		newlevel = (newlevel * NVIDIA_BRIGHT_SCALE) + NVIDIA_BRIGHT_MIN;

	pmc_reg = bus_read_stream_4(sc->sc_memr, NVIDIA_PMC_OFF) & 0xffff;
	pmc_reg |= NVIDIA_PMC_BL_EN | (newlevel << NVIDIA_PMC_BL_SHIFT);
	bus_write_stream_4(sc->sc_memr, NVIDIA_PMC_OFF, pmc_reg);

	return (0);
}

static int
nvbl_getlevel(struct nvbl_softc *sc)
{
	uint16_t level;

	level = bus_read_stream_2(sc->sc_memr, NVIDIA_PMC_OFF) & 0x7fff;

	if (level  < NVIDIA_BRIGHT_MIN)
		return 0;

	level = (level - NVIDIA_BRIGHT_MIN) / NVIDIA_BRIGHT_SCALE;

	return (level);
}

static int
nvbl_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct nvbl_softc *sc;
	int newlevel, error;

	sc = arg1;

	newlevel = nvbl_getlevel(sc);

	error = sysctl_handle_int(oidp, &newlevel, 0, req);

	if (error || !req->newptr)
		return (error);

	return (nvbl_setlevel(sc, newlevel));
}
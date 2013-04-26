
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

/* From the xf86-video-ati driver's radeon_reg.h */
#define RADEON_LVDS_GEN_CNTL         0x02d0
#define  RADEON_LVDS_ON               (1   <<  0)
#define  RADEON_LVDS_DISPLAY_DIS      (1   <<  1)
#define  RADEON_LVDS_PANEL_TYPE       (1   <<  2)
#define  RADEON_LVDS_PANEL_FORMAT     (1   <<  3)
#define  RADEON_LVDS_RST_FM           (1   <<  6)
#define  RADEON_LVDS_EN               (1   <<  7)
#define  RADEON_LVDS_BL_MOD_LEVEL_SHIFT 8
#define  RADEON_LVDS_BL_MOD_LEVEL_MASK (0xff << 8)
#define  RADEON_LVDS_BL_MOD_EN        (1   << 16)
#define  RADEON_LVDS_DIGON            (1   << 18)
#define  RADEON_LVDS_BLON             (1   << 19)

struct atibl_softc {
	device_t	 dev;
	struct resource *sc_memr;
};

static void atibl_identify(driver_t *driver, device_t parent);
static int atibl_probe(device_t dev);
static int atibl_attach(device_t dev);
static int atibl_setlevel(struct atibl_softc *sc, int newlevel);
static int atibl_getlevel(struct atibl_softc *sc);
static int atibl_sysctl(SYSCTL_HANDLER_ARGS);

static device_method_t atibl_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify, atibl_identify),
	DEVMETHOD(device_probe, atibl_probe),
	DEVMETHOD(device_attach, atibl_attach),
	{0, 0},
};

static driver_t	atibl_driver = {
	"backlight",
	atibl_methods,
	sizeof(struct atibl_softc)
};

static devclass_t atibl_devclass;

DRIVER_MODULE(atibl, vgapci, atibl_driver, atibl_devclass, 0, 0);

static void
atibl_identify(driver_t *driver, device_t parent)
{
	if (device_find_child(parent, "backlight", -1) == NULL)
		device_add_child(parent, "backlight", -1);
}

static int
atibl_probe(device_t dev)
{
	char		control[8];
	phandle_t	handle;

	handle = OF_finddevice("mac-io/backlight");

	if (handle == -1)
		return (ENXIO);

	if (OF_getprop(handle, "backlight-control", &control, sizeof(control)) < 0)
		return (ENXIO);

	if (strcmp(control, "ati") != 0)
		return (ENXIO);

	device_set_desc(dev, "PowerBook backlight for ATI graphics");

	return (0);
}

static int
atibl_attach(device_t dev)
{
	struct atibl_softc	*sc;
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid *tree;
	int			 rid;

	sc = device_get_softc(dev);

	rid = 0x18;	/* BAR[2], for the MMIO register */
	sc->sc_memr = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid,
			RF_ACTIVE | RF_SHAREABLE);
	if (sc->sc_memr == NULL) {
		device_printf(dev, "Could not alloc mem resource!\n");
		return (ENXIO);
	}

	ctx = device_get_sysctl_ctx(dev);
	tree = device_get_sysctl_tree(dev);

	SYSCTL_ADD_PROC(ctx, SYSCTL_CHILDREN(tree), OID_AUTO,
			"level", CTLTYPE_INT | CTLFLAG_RW, sc, 0,
			atibl_sysctl, "I", "Backlight level (0-100)");

	return (0);
}

static int
atibl_setlevel(struct atibl_softc *sc, int newlevel)
{
	uint32_t lvds_gen_cntl;

	if (newlevel > 100)
		newlevel = 100;

	if (newlevel < 0)
		newlevel = 0;

	newlevel = (newlevel * 5) / 2 + 5;
	lvds_gen_cntl = bus_read_4(sc->sc_memr, RADEON_LVDS_GEN_CNTL);
	lvds_gen_cntl |= RADEON_LVDS_BL_MOD_EN;
	lvds_gen_cntl &= ~RADEON_LVDS_BL_MOD_LEVEL_MASK;
	lvds_gen_cntl |= (newlevel << RADEON_LVDS_BL_MOD_LEVEL_SHIFT) &
		RADEON_LVDS_BL_MOD_LEVEL_MASK;
	bus_write_4(sc->sc_memr, RADEON_LVDS_GEN_CNTL, lvds_gen_cntl);

	return (0);
}

static int
atibl_getlevel(struct atibl_softc *sc)
{
	uint32_t	lvds_gen_cntl;
	int			level;

	lvds_gen_cntl = bus_read_4(sc->sc_memr, RADEON_LVDS_GEN_CNTL);

	level = ((lvds_gen_cntl & RADEON_LVDS_BL_MOD_LEVEL_MASK) >>
			RADEON_LVDS_BL_MOD_LEVEL_SHIFT);
	level = ((level - 5) * 2) / 5;

	return (level);
}

static int
atibl_sysctl(SYSCTL_HANDLER_ARGS)
{
	struct atibl_softc *sc;
	int newlevel, error;

	sc = arg1;

	newlevel = atibl_getlevel(sc);

	error = sysctl_handle_int(oidp, &newlevel, 0, req);

	if (error || !req->newptr)
		return (error);

	return (atibl_setlevel(sc, newlevel));
}
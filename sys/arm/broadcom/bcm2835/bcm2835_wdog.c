
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
#include <sys/watchdog.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/rman.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <machine/bus.h>
#include <machine/cpufunc.h>
#include <machine/machdep.h>
#include <machine/fdt.h>

#include <arm/broadcom/bcm2835/bcm2835_wdog.h>

#define	BCM2835_PASWORD		0x5a

#define BCM2835_WDOG_RESET	0
#define BCM2835_PASSWORD_MASK	0xff000000
#define BCM2835_PASSWORD_SHIFT	24
#define BCM2835_WDOG_TIME_MASK	0x000fffff
#define BCM2835_WDOG_TIME_SHIFT	0

#define	READ(_sc, _r) bus_space_read_4((_sc)->bst, (_sc)->bsh, (_r))
#define	WRITE(_sc, _r, _v) bus_space_write_4((_sc)->bst, (_sc)->bsh, (_r), (_v))

#define BCM2835_RSTC_WRCFG_CLR		0xffffffcf
#define BCM2835_RSTC_WRCFG_SET		0x00000030
#define BCM2835_RSTC_WRCFG_FULL_RESET	0x00000020
#define BCM2835_RSTC_RESET		0x00000102

#define	BCM2835_RSTC_REG	0x00
#define	BCM2835_RSTS_REG	0x04
#define	BCM2835_WDOG_REG	0x08

static struct bcmwd_softc *bcmwd_lsc = NULL;

struct bcmwd_softc {
	device_t		dev;
	struct resource *	res;
	bus_space_tag_t		bst;
	bus_space_handle_t	bsh;
	int			wdog_armed;
	int			wdog_period;
	char			wdog_passwd;
};

#ifdef notyet
static void bcmwd_watchdog_fn(void *private, u_int cmd, int *error);
#endif

static int
bcmwd_probe(device_t dev)
{

	if (ofw_bus_is_compatible(dev, "broadcom,bcm2835-wdt")) {
		device_set_desc(dev, "BCM2708/2835 Watchdog");
		return (BUS_PROBE_DEFAULT);
	}

	return (ENXIO);
}

static int
bcmwd_attach(device_t dev)
{
	struct bcmwd_softc *sc;
	int rid;

	if (bcmwd_lsc != NULL)
		return (ENXIO);

	sc = device_get_softc(dev);
	sc->wdog_period = 7;
	sc->wdog_passwd = BCM2835_PASWORD;
	sc->wdog_armed = 0;
	sc->dev = dev;

	rid = 0;
	sc->res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, RF_ACTIVE);
	if (sc->res == NULL) {
		device_printf(dev, "could not allocate memory resource\n");
		return (ENXIO);
	}

	sc->bst = rman_get_bustag(sc->res);
	sc->bsh = rman_get_bushandle(sc->res);

	bcmwd_lsc = sc;
#ifdef notyet
	EVENTHANDLER_REGISTER(watchdog_list, bcmwd_watchdog_fn, sc, 0);
#endif
	return (0);
}

#ifdef notyet
static void
bcmwd_watchdog_fn(void *private, u_int cmd, int *error)
{
	/* XXX: not yet */
}
#endif

void
bcmwd_watchdog_reset()
{

	if (bcmwd_lsc == NULL)
		return;

	WRITE(bcmwd_lsc, BCM2835_WDOG_REG,
	    (BCM2835_PASWORD << BCM2835_PASSWORD_SHIFT) | 10);

	WRITE(bcmwd_lsc, BCM2835_RSTC_REG,
	    (READ(bcmwd_lsc, BCM2835_RSTC_REG) & BCM2835_RSTC_WRCFG_CLR) |
		(BCM2835_PASWORD << BCM2835_PASSWORD_SHIFT) |
		BCM2835_RSTC_WRCFG_FULL_RESET);
}

static device_method_t bcmwd_methods[] = {
	DEVMETHOD(device_probe, bcmwd_probe),
	DEVMETHOD(device_attach, bcmwd_attach),

	DEVMETHOD_END
};

static driver_t bcmwd_driver = {
	"bcmwd",
	bcmwd_methods,
	sizeof(struct bcmwd_softc),
};
static devclass_t bcmwd_devclass;

DRIVER_MODULE(bcmwd, simplebus, bcmwd_driver, bcmwd_devclass, 0, 0);
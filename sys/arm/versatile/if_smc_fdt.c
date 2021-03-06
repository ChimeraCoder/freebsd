
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
#include <sys/kernel.h>
#include <sys/socket.h>

#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/if_media.h>

#include <dev/smc/if_smcvar.h>

#include <dev/mii/mii.h>
#include <dev/mii/miivar.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include "miibus_if.h"

static int		smc_fdt_probe(device_t);
static int		smc_fdt_attach(device_t);
static int		smc_fdt_detach(device_t);

static int
smc_fdt_probe(device_t dev)
{
	struct	smc_softc *sc;

	if (ofw_bus_is_compatible(dev, "smsc,lan91c111")) {
		sc = device_get_softc(dev);
		sc->smc_usemem = 1;

		if (smc_probe(dev) != 0) {
			return (ENXIO);
		}

		return (0);
	}

	return (ENXIO);
}

static int
smc_fdt_attach(device_t dev)
{
	int	err;
 	struct	smc_softc *sc;

	sc = device_get_softc(dev);

	err = smc_attach(dev);
	if (err) {
		return (err);
	}

	return (0);
}

static int
smc_fdt_detach(device_t dev)
{

	smc_detach(dev);

	return (0);
}

static device_method_t smc_fdt_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		smc_fdt_probe),
	DEVMETHOD(device_attach,	smc_fdt_attach),
	DEVMETHOD(device_detach,	smc_fdt_detach),

	/* MII interface */
	DEVMETHOD(miibus_readreg,	smc_miibus_readreg),
	DEVMETHOD(miibus_writereg,	smc_miibus_writereg),
	DEVMETHOD(miibus_statchg,	smc_miibus_statchg),

	{ 0, 0 }
};

static driver_t smc_fdt_driver = {
	"smc",
	smc_fdt_methods,
	sizeof(struct smc_softc),
};

extern devclass_t smc_devclass;

DRIVER_MODULE(smc, simplebus, smc_fdt_driver, smc_devclass, 0, 0);
DRIVER_MODULE(miibus, smc, miibus_driver, miibus_devclass, 0, 0);
MODULE_DEPEND(smc, fdt, 1, 1, 1);
MODULE_DEPEND(smc, ether, 1, 1, 1);
MODULE_DEPEND(smc, miibus, 1, 1, 1);
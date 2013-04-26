
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

/*
 * Driver to swallow up memory ranges reserved by CFE platform firmware.
 * CFE on Sentry5 doesn't specify reserved ranges, so this is not useful
 * at the present time.
 * TODO: Don't attach this off nexus.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/socket.h>

#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/cfe/cfe_api.h>
#include <dev/cfe/cfe_error.h>

#define MAX_CFE_RESERVATIONS 16

struct cferes_softc {
	int		 rnum;
	int		 rid[MAX_CFE_RESERVATIONS];
	struct resource	*res[MAX_CFE_RESERVATIONS];
};

static int
cferes_probe(device_t dev)
{

	return (0);
}

static int
cferes_attach(device_t dev)
{

	return (0);
}

static void
cferes_identify(driver_t* driver, device_t parent)
{
	device_t		 child;
	int			 i;
	struct resource		*res;
	int			 result;
	int			 rid;
	struct cferes_softc	*sc;
	uint64_t		 addr, len, type;

	child = BUS_ADD_CHILD(parent, 100, "cferes", -1);
	device_set_driver(child, driver);
	sc = device_get_softc(child);

	sc->rnum = 0;
	for (i = 0; i < ~0U; i++) {
		result = cfe_enummem(i, CFE_FLG_FULL_ARENA, &addr, &len, &type);
		if (result < 0)
			break;
		if (type != CFE_MI_RESERVED) {
			if (bootverbose)
			printf("%s: skipping non reserved range 0x%0jx(%jd)\n",
			    device_getnameunit(child),
			    (uintmax_t)addr, (uintmax_t)len);
			continue;
		}

		bus_set_resource(child, SYS_RES_MEMORY, sc->rnum, addr, len);
		rid = sc->rnum;
		res = bus_alloc_resource_any(child, SYS_RES_MEMORY, &rid, 0);
		if (res == NULL) {
			bus_delete_resource(child, SYS_RES_MEMORY, sc->rnum);
			continue;
		}
		sc->rid[sc->rnum] = rid;
		sc->res[sc->rnum] = res;

		sc->rnum++;
		if (sc->rnum == MAX_CFE_RESERVATIONS)
			break;
	}

	if (sc->rnum == 0) {
		device_delete_child(parent, child);
		return;
	}

	device_set_desc(child, "CFE reserved memory");
}

static int
cferes_detach(device_t dev)
{
	int			i;
	struct cferes_softc	*sc = device_get_softc(dev);

	for (i = 0; i < sc->rnum; i++) {
		bus_release_resource(dev, SYS_RES_MEMORY, sc->rid[i],
		    sc->res[i]);
	}

	return (0);
}

static device_method_t cferes_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	cferes_identify),
	DEVMETHOD(device_probe,		cferes_probe),
	DEVMETHOD(device_attach,	cferes_attach),
	DEVMETHOD(device_detach,	cferes_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD(device_suspend,	bus_generic_suspend),
	DEVMETHOD(device_resume,	bus_generic_resume),
	{ 0, 0 }
};

static driver_t cferes_driver = {
	"cferes",
	cferes_methods,
	sizeof (struct cferes_softc)
};

static devclass_t cferes_devclass;

DRIVER_MODULE(cfe, nexus, cferes_driver, cferes_devclass, 0, 0);
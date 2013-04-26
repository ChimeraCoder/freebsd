
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/reboot.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <pc98/pc98/canbusvars.h>
#include "canbus_if.h"


/* canbepm softc */
struct canbepm_softc {
	device_t canbepm_dev;			/* canbepm device */

	eventhandler_tag canbepm_tag;		/* event handler tag */
};


static void	canbepm_soft_off (void *, int);
static void	canbepm_identify (driver_t *, device_t);
static int	canbepm_probe (device_t);
static int	canbepm_attach (device_t);
static int	canbepm_detach (device_t);


static device_method_t canbepm_methods[] = { 
	DEVMETHOD(device_identify,	canbepm_identify),
	DEVMETHOD(device_probe,		canbepm_probe),
	DEVMETHOD(device_attach,	canbepm_attach),
	DEVMETHOD(device_detach,	canbepm_detach),
	{0, 0}
};

static driver_t canbepm_driver = {
	"canbepm",
	canbepm_methods,
	sizeof(struct canbepm_softc),
};

devclass_t canbepm_devclass;
DRIVER_MODULE(canbepm, canbus, canbepm_driver, canbepm_devclass, 0, 0);
MODULE_DEPEND(canbepm, canbus, 1, 1, 1);


static void
canbepm_soft_off (void *data, int howto)
{
	struct canbepm_softc *sc = data;
	u_int8_t poweroff_data[] = CANBE_POWEROFF_DATA;

	if (!(howto & RB_POWEROFF))
		return;

	CANBUS_WRITE_MULTI(device_get_parent(sc->canbepm_dev), sc->canbepm_dev,
	    CANBE_POWER_CTRL, sizeof (poweroff_data), poweroff_data);
}


static void
canbepm_identify(driver_t *drv, device_t parent)
{
	if (device_find_child(parent, "canbepm", 0) == NULL) {
		if (BUS_ADD_CHILD(parent, 33, "canbepm", 0) == NULL)
			device_printf(parent, "canbepm cannot attach\n");
	}
}


static int
canbepm_probe(device_t dev)
{
	device_set_desc(dev, "CanBe Power Management Controller");

	return (0);	
}

static int
canbepm_attach(device_t dev)
{
	struct canbepm_softc *sc = device_get_softc(dev);

	/* eventhandler regist */
	sc->canbepm_tag = EVENTHANDLER_REGISTER(
	    shutdown_final, canbepm_soft_off, sc, SHUTDOWN_PRI_LAST);

	sc->canbepm_dev = dev;

	return (0);
}


static int
canbepm_detach(device_t dev)
{
	struct canbepm_softc *sc = device_get_softc(dev);

	/* eventhandler deregist */
	EVENTHANDLER_DEREGISTER(shutdown_final, sc->canbepm_tag);
	BUS_CHILD_DETACHED(device_get_parent(dev), dev);

	return (0);
}

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
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/bus.h>

#include <machine/bus.h>

#include <dev/ofw/openfirm.h>
#include <machine/ofw_machdep.h>

#include <dev/adb/adb.h>

#define ABTN_HANDLER_ID 31

struct abtn_softc {
	device_t sc_dev;

	int handler_id;
};

static int abtn_probe(device_t dev);
static int abtn_attach(device_t dev);
static u_int abtn_receive_packet(device_t dev, u_char status, 
    u_char command, u_char reg, int len, u_char *data);

static device_method_t abtn_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,         abtn_probe),
        DEVMETHOD(device_attach,        abtn_attach),
        DEVMETHOD(device_shutdown,      bus_generic_shutdown),
        DEVMETHOD(device_suspend,       bus_generic_suspend),
        DEVMETHOD(device_resume,        bus_generic_resume),

	/* ADB interface */
	DEVMETHOD(adb_receive_packet,	abtn_receive_packet),

	{ 0, 0 }
};

static driver_t abtn_driver = {
	"abtn",
	abtn_methods,
	sizeof(struct abtn_softc),
};

static devclass_t abtn_devclass;

DRIVER_MODULE(abtn, adb, abtn_driver, abtn_devclass, 0, 0);

static int
abtn_probe(device_t dev)
{
	uint8_t type;

	type = adb_get_device_type(dev);

	if (type != ADB_DEVICE_MISC)
		return (ENXIO);

	device_set_desc(dev, "ADB Brightness/Volume/Eject Buttons");
	return (0);
}

static int
abtn_attach(device_t dev) 
{
	struct abtn_softc *sc;

	sc = device_get_softc(dev);
	sc->sc_dev = dev;

	sc->handler_id = adb_get_device_handler(dev);

	return 0;
}

static u_int
abtn_receive_packet(device_t dev, u_char status, 
    u_char command, u_char reg, int len, u_char *data)
{
	u_int cmd;

	cmd = data[0];

	switch (cmd) {
	case 0x0a:	/* decrease brightness */
		if (devctl_process_running())
			devctl_notify("PMU", "keys", "brightness",
			    "notify=down");
		break;

	case 0x09:	/* increase brightness */
		if (devctl_process_running())
			devctl_notify("PMU", "keys", "brightness", "notify=up");
		break;

	case 0x08:	/* mute */
	case 0x01:	/* mute, AV hardware */
		if (devctl_process_running())
			devctl_notify("PMU", "keys", "mute", NULL);
		break;
	case 0x07:	/* decrease volume */
	case 0x02:	/* decrease volume, AV hardware */
		if (devctl_process_running())
			devctl_notify("PMU", "keys", "volume", "notify=down");
		break;
	case 0x06:	/* increase volume */
	case 0x03:	/* increase volume, AV hardware */
		if (devctl_process_running())
			devctl_notify("PMU", "keys", "volume", "notify=up");
		break;
	case 0x0c:	/* mirror display key */
		/* Need callback to do something with this */
		break;
	case 0x0b:	/* eject tray */
		if (devctl_process_running())
			devctl_notify("PMU", "keys", "eject", NULL);
	case 0x7f:	/* numlock */
		/* Need callback to do something with this */
		break;

	default:
#ifdef DEBUG
		if ((cmd & ~0x7f) == 0)
			device_printf(dev, "unknown ADB button 0x%x\n", cmd);
#endif
		break;
	}
	return 0;
}
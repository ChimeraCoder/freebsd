
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
#include <sys/condvar.h>
#include <sys/eventhandler.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/selinfo.h>

#include <dev/smbus/smbconf.h>
#include <dev/smbus/smbus.h>
#include <dev/smbus/smb.h>

#include "smbus_if.h"

#ifdef LOCAL_MODULE
#include <ipmivars.h>
#else
#include <dev/ipmi/ipmivars.h>
#endif

static void ipmi_smbus_identify(driver_t *driver, device_t parent);
static int ipmi_smbus_probe(device_t dev);
static int ipmi_smbus_attach(device_t dev);

static void
ipmi_smbus_identify(driver_t *driver, device_t parent)
{
	struct ipmi_get_info info;

	if (ipmi_smbios_identify(&info) && info.iface_type == SSIF_MODE &&
	    device_find_child(parent, "ipmi", -1) == NULL)
		BUS_ADD_CHILD(parent, 0, "ipmi", -1);
}

static int
ipmi_smbus_probe(device_t dev)
{

	device_set_desc(dev, "IPMI System Interface");
	return (BUS_PROBE_DEFAULT);
}

static int
ipmi_smbus_attach(device_t dev)
{
	struct ipmi_softc *sc = device_get_softc(dev);
	struct ipmi_get_info info;
	int error;

	/* This should never fail. */
	if (!ipmi_smbios_identify(&info))
		return (ENXIO);

	if (info.iface_type != SSIF_MODE) {
		device_printf(dev, "No SSIF IPMI interface found\n");
		return (ENXIO);
	}

	sc->ipmi_dev = dev;

	if (info.irq != 0) {
		sc->ipmi_irq_rid = 0;
		sc->ipmi_irq_res = bus_alloc_resource(dev, SYS_RES_IRQ,
		    &sc->ipmi_irq_rid, info.irq, info.irq, 1,
		    RF_SHAREABLE | RF_ACTIVE);
	}

	device_printf(dev, "SSIF mode found at address 0x%llx on %s\n",
	    (long long)info.address, device_get_name(device_get_parent(dev)));
	error = ipmi_ssif_attach(sc, device_get_parent(dev), info.address);
	if (error)
		goto bad;

	error = ipmi_attach(dev);
	if (error)
		goto bad;

	return (0);
bad:
	ipmi_release_resources(dev);
	return (error);
}

static device_method_t ipmi_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	ipmi_smbus_identify),
	DEVMETHOD(device_probe,		ipmi_smbus_probe),
	DEVMETHOD(device_attach,	ipmi_smbus_attach),
	DEVMETHOD(device_detach,	ipmi_detach),
	{ 0, 0 }
};

static driver_t ipmi_smbus_driver = {
	"ipmi",
	ipmi_methods,
	sizeof(struct ipmi_softc)
};

DRIVER_MODULE(ipmi_smbus, smbus, ipmi_smbus_driver, ipmi_devclass, 0, 0);
MODULE_DEPEND(ipmi_smbus, smbus, SMBUS_MINVER, SMBUS_PREFVER, SMBUS_MAXVER);
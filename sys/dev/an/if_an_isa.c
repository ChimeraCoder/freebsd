
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

#include "opt_inet.h"

#ifdef INET
#define ANCACHE
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/sockio.h>
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <sys/socket.h>

#include <sys/module.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_media.h>

#include <isa/isavar.h>
#include <isa/pnpvar.h>

#include <dev/an/if_aironet_ieee.h>
#include <dev/an/if_anreg.h>

static struct isa_pnp_id an_ids[] = {
	{ 0x0100ec06, "Aironet ISA4500/ISA4800" },
	{ 0, NULL }
};

static int an_probe_isa(device_t);
static int an_attach_isa(device_t);

static int
an_probe_isa(device_t dev)
{
	int			error = 0;

	error = ISA_PNP_PROBE(device_get_parent(dev), dev, an_ids);
	if (error == ENXIO)
		return(error);

	error = an_probe(dev);
	an_release_resources(dev);
	if (error == 0)
		return (ENXIO);

	error = an_alloc_irq(dev, 0, 0);
	an_release_resources(dev);
	if (!error)
		device_set_desc(dev, "Aironet ISA4500/ISA4800");
	return (error);
}

static int
an_attach_isa(device_t dev)
{
	struct an_softc *sc = device_get_softc(dev);
	int flags = device_get_flags(dev);
	int error;

	an_alloc_port(dev, sc->port_rid, 1);
	an_alloc_irq(dev, sc->irq_rid, 0);

	sc->an_dev = dev;

	error = an_attach(sc, flags);
	if (error) {
		an_release_resources(dev);
		return (error);
	}

	error = bus_setup_intr(dev, sc->irq_res, INTR_TYPE_NET,
			       NULL, an_intr, sc, &sc->irq_handle);
	if (error) {
		an_release_resources(dev);
		return (error);
	}
	return (0);
}

static device_method_t an_isa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		an_probe_isa),
	DEVMETHOD(device_attach,	an_attach_isa),
	DEVMETHOD(device_detach,	an_detach),
	DEVMETHOD(device_shutdown,	an_shutdown),
	{ 0, 0 }
};

static driver_t an_isa_driver = {
	"an",
	an_isa_methods,
	sizeof(struct an_softc)
};

static devclass_t an_isa_devclass;

DRIVER_MODULE(an, isa, an_isa_driver, an_isa_devclass, 0, 0);
MODULE_DEPEND(an, isa, 1, 1, 1);
MODULE_DEPEND(an, wlan, 1, 1, 1);

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
#include <sys/socket.h>
#include <sys/kernel.h>

#include <sys/module.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <machine/resource.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/if_media.h>

#include <dev/an/if_aironet_ieee.h>
#include <dev/an/if_anreg.h>

#include <dev/pccard/pccardvar.h>

#include "pccarddevs.h"
#include "card_if.h"

/*
 * Support for PCMCIA cards.
 */
static int  an_pccard_probe(device_t);
static int  an_pccard_attach(device_t);

static device_method_t an_pccard_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		an_pccard_probe),
	DEVMETHOD(device_attach,	an_pccard_attach),
	DEVMETHOD(device_detach,	an_detach),
	DEVMETHOD(device_shutdown,	an_shutdown),

	{ 0, 0 }
};

static driver_t an_pccard_driver = {
	"an",
	an_pccard_methods,
	sizeof(struct an_softc)
};

static devclass_t an_pccard_devclass;

DRIVER_MODULE(an, pccard, an_pccard_driver, an_pccard_devclass, 0, 0);
MODULE_DEPEND(an, wlan, 1, 1, 1);

static const struct pccard_product an_pccard_products[] = {
	PCMCIA_CARD(AIRONET, PC4800),
	PCMCIA_CARD(AIRONET, PC4500),
	PCMCIA_CARD(AIRONET, 350),
	PCMCIA_CARD(XIRCOM, CWE1130), 
	{ NULL }
};

static int
an_pccard_probe(device_t dev)
{
	const struct pccard_product *pp;

	if ((pp = pccard_product_lookup(dev, an_pccard_products,
	    sizeof(an_pccard_products[0]), NULL)) != NULL) {
		if (pp->pp_name != NULL)
			device_set_desc(dev, pp->pp_name);
		return (0);
	}
	return (ENXIO);
}

static int
an_pccard_attach(device_t dev)
{
	struct an_softc *sc = device_get_softc(dev);
	int flags = device_get_flags(dev);
	int     error;

	error = an_probe(dev); /* 0 is failure for now */
	if (error == 0) {
		error = ENXIO;
		goto fail;
	}
	error = an_alloc_irq(dev, 0, 0);
	if (error != 0)
		goto fail;

	an_alloc_irq(dev, sc->irq_rid, 0);

	sc->an_dev = dev;

	error = an_attach(sc, flags);
	if (error)
		goto fail;
	
	/*
	 * Must setup the interrupt after the an_attach to prevent racing.
	 */
	error = bus_setup_intr(dev, sc->irq_res, INTR_TYPE_NET,
			       NULL, an_intr, sc, &sc->irq_handle);
fail:
	if (error)
		an_release_resources(dev);
	return (error);
}

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
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/selinfo.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/cmx/cmxvar.h>

#include <dev/pccard/pccardvar.h>
#include <dev/pccard/pccard_cis.h>

#include "pccarddevs.h"

static const struct pccard_product cmx_pccard_products[] = {
	PCMCIA_CARD(OMNIKEY, CM4040),
	{ NULL }
};

/*
 * Probe for the card.
 */
static int
cmx_pccard_probe(device_t dev)
{
	const struct pccard_product *pp;
	if ((pp = pccard_product_lookup(dev, cmx_pccard_products,
	    sizeof(cmx_pccard_products[0]), NULL)) != NULL) {
		if (pp->pp_name != NULL)
			device_set_desc(dev, pp->pp_name);
		return 0;
	}
	return EIO;
}

/*
 * Attach to the pccard, and call bus independant attach and
 * resource allocation routines.
 */
static int
cmx_pccard_attach(device_t dev)
{
	int rv = 0;
	cmx_init_softc(dev);

	if ((rv = cmx_alloc_resources(dev)) != 0) {
		device_printf(dev, "cmx_alloc_resources() failed!\n");
		cmx_release_resources(dev);
		return rv;
	}

	if ((rv = cmx_attach(dev)) != 0) {
		device_printf(dev, "cmx_attach() failed!\n");
		cmx_release_resources(dev);
		return rv;
	}

	device_printf(dev, "attached\n");
	return 0;
}

static device_method_t cmx_pccard_methods[] = {
	DEVMETHOD(device_probe, cmx_pccard_probe),
	DEVMETHOD(device_attach, cmx_pccard_attach),
	DEVMETHOD(device_detach, cmx_detach),

	{ 0, 0 }
};

static driver_t cmx_pccard_driver = {
	"cmx",
	cmx_pccard_methods,
	sizeof(struct cmx_softc),
};

DRIVER_MODULE(cmx, pccard, cmx_pccard_driver, cmx_devclass, 0, 0);
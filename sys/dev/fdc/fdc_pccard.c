
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
#include <sys/bio.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/systm.h>
#include <machine/bus.h>

#include <dev/fdc/fdcvar.h>
#include <dev/pccard/pccardvar.h>
#include "pccarddevs.h"

static int fdc_pccard_probe(device_t);
static int fdc_pccard_attach(device_t);

static const struct pccard_product fdc_pccard_products[] = {
	PCMCIA_CARD(YEDATA, EXTERNAL_FDD),
};
	
static int
fdc_pccard_alloc_resources(device_t dev, struct fdc_data *fdc)
{
	struct resource *res;
	int rid, i;

	rid = 0;
	res = bus_alloc_resource(dev, SYS_RES_IOPORT, &rid, 0ul, ~0ul, 1,
	    RF_ACTIVE);
	if (res == NULL) {
		device_printf(dev, "cannot alloc I/O port range\n");
		return (ENXIO);
	}
	for (i = 0; i < FDC_MAXREG; i++) {
		fdc->resio[i] = res;
		fdc->ridio[i] = rid;
		fdc->ioff[i] = i;
		fdc->ioh[i] = rman_get_bushandle(res);
	}
	fdc->iot = rman_get_bustag(res);

	fdc->rid_irq = 0;
	fdc->res_irq = bus_alloc_resource_any(dev, SYS_RES_IRQ, &fdc->rid_irq,
	    RF_ACTIVE | RF_SHAREABLE);
	if (fdc->res_irq == NULL) {
		device_printf(dev, "cannot reserve interrupt line\n");
		return (ENXIO);
	}
	return (0);
}

static int
fdc_pccard_probe(device_t dev)
{
	if (pccard_product_lookup(dev, fdc_pccard_products,
	    sizeof(fdc_pccard_products[0]), NULL) != NULL) {
		device_set_desc(dev, "PC Card Floppy");
		return (0);
	}
	return (ENXIO);
}

static int
fdc_pccard_attach(device_t dev)
{
	int error;
	struct	fdc_data *fdc;
	device_t child;

	fdc = device_get_softc(dev);
	fdc->flags = FDC_NODMA | FDC_NOFAST;
	fdc->fdct = FDC_NE765;
	error = fdc_pccard_alloc_resources(dev, fdc);
	if (error == 0)
		error = fdc_attach(dev);
	if (error == 0) {
		child = fdc_add_child(dev, "fd", -1);
		device_set_flags(child, 0x24);
		error = bus_generic_attach(dev);
	}
	if (error)
		fdc_release_resources(fdc);
	return (error);
}

static device_method_t fdc_pccard_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		fdc_pccard_probe),
	DEVMETHOD(device_attach,	fdc_pccard_attach),
	DEVMETHOD(device_detach,	fdc_detach),
	DEVMETHOD(device_shutdown,	bus_generic_shutdown),
	DEVMETHOD(device_suspend,	bus_generic_suspend),
	DEVMETHOD(device_resume,	bus_generic_resume),

	/* Bus interface */
	DEVMETHOD(bus_print_child,	fdc_print_child),
	DEVMETHOD(bus_read_ivar,	fdc_read_ivar),
	DEVMETHOD(bus_write_ivar,       fdc_write_ivar),
	/* Our children never use any other bus interface methods. */

	{ 0, 0 }
};

static driver_t fdc_pccard_driver = {
	"fdc",
	fdc_pccard_methods,
	sizeof(struct fdc_data)
};

DRIVER_MODULE(fdc, pccard, fdc_pccard_driver, fdc_devclass, 0, 0);
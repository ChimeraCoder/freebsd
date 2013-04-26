
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
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/systm.h>

#include <machine/bus.h>

#include <pc98/cbus/cbus.h>
#include <pc98/cbus/fdcreg.h>
#include <pc98/cbus/fdcvar.h>

#include <isa/isavar.h>

static bus_addr_t fdc_iat[] = {0, 2, 4};

static int
fdc_cbus_alloc_resources(device_t dev, struct fdc_data *fdc)
{
	int rid;

	fdc->fdc_dev = dev;
	fdc->rid_ioport = 0;
	fdc->rid_irq = 0;
	fdc->rid_drq = 0;
	fdc->res_irq = 0;
	fdc->res_drq = 0;

	fdc->res_ioport = isa_alloc_resourcev(dev, SYS_RES_IOPORT,
					      &fdc->rid_ioport, fdc_iat,
					      3, RF_ACTIVE);
	if (fdc->res_ioport == 0) {
		device_printf(dev, "cannot reserve I/O port range\n");
		return ENXIO;
	}
	isa_load_resourcev(fdc->res_ioport, fdc_iat, 3);
	fdc->portt = rman_get_bustag(fdc->res_ioport);
	fdc->porth = rman_get_bushandle(fdc->res_ioport);

	rid = 3;
	bus_set_resource(dev, SYS_RES_IOPORT, rid, IO_FDPORT, 1);
	fdc->res_fdsio = bus_alloc_resource_any(dev, SYS_RES_IOPORT, &rid,
						RF_ACTIVE);
	if (fdc->res_fdsio == 0)
		return ENXIO;
	fdc->sc_fdsiot = rman_get_bustag(fdc->res_fdsio);
	fdc->sc_fdsioh = rman_get_bushandle(fdc->res_fdsio);

	rid = 4;
	bus_set_resource(dev, SYS_RES_IOPORT, rid, 0x4be, 1);
	fdc->res_fdemsio = bus_alloc_resource_any(dev, SYS_RES_IOPORT, &rid,
						  RF_ACTIVE);
	if (fdc->res_fdemsio == 0)
		return ENXIO;
	fdc->sc_fdemsiot = rman_get_bustag(fdc->res_fdemsio);
	fdc->sc_fdemsioh = rman_get_bushandle(fdc->res_fdemsio);

	fdc->res_irq = bus_alloc_resource_any(dev, SYS_RES_IRQ, &fdc->rid_irq,
					      RF_ACTIVE);
	if (fdc->res_irq == 0) {
		device_printf(dev, "cannot reserve interrupt line\n");
		return ENXIO;
	}

	if ((fdc->flags & FDC_NODMA) == 0) {
		fdc->res_drq = bus_alloc_resource_any(dev, SYS_RES_DRQ,
						      &fdc->rid_drq, RF_ACTIVE);
		if (fdc->res_drq == 0) {
			device_printf(dev, "cannot reserve DMA request line\n");
			return ENXIO;
		}
		fdc->dmachan = rman_get_start(fdc->res_drq);
	}

	return 0;
}

static int
fdc_cbus_probe(device_t dev)
{
	int	error;
	struct	fdc_data *fdc;

	fdc = device_get_softc(dev);

	/* Check pnp ids */
	if (isa_get_vendorid(dev))
		return (ENXIO);

	/* Attempt to allocate our resources for the duration of the probe */
	error = fdc_cbus_alloc_resources(dev, fdc);
	if (!error)
		error = fdc_initial_reset(fdc);

	fdc_release_resources(fdc);
	return (error);
}

static int
fdc_cbus_attach(device_t dev)
{
	struct	fdc_data *fdc;
	int error;

	fdc = device_get_softc(dev);

	if ((error = fdc_cbus_alloc_resources(dev, fdc)) != 0 ||
	    (error = fdc_attach(dev)) != 0 ||
	    (error = fdc_hints_probe(dev)) != 0) {
		fdc_release_resources(fdc);
		return (error);
	}

	return (0);
}

static device_method_t fdc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		fdc_cbus_probe),
	DEVMETHOD(device_attach,	fdc_cbus_attach),
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

static driver_t fdc_driver = {
	"fdc",
	fdc_methods,
	sizeof(struct fdc_data)
};

DRIVER_MODULE(fdc, isa, fdc_driver, fdc_devclass, 0, 0);
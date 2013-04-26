
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
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/malloc.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pccard/pccardvar.h>

#include <dev/puc/puc_cfg.h>
#include <dev/puc/puc_bfe.h>

/* http://www.argosy.com.tw/product/sp320.htm */
const struct puc_cfg puc_pccard_rscom = {
	0, 0, 0, 0,
	"ARGOSY SP320 Dual port serial PCMCIA",
	DEFAULT_RCLK,
	PUC_PORT_2S, 0, 1, 0,
};

static int
puc_pccard_probe(device_t dev)
{
	const char *vendor, *product;
	int error;

	error = pccard_get_vendor_str(dev, &vendor);
	if (error)
		return(error);
	error = pccard_get_product_str(dev, &product);
	if (error)
		return(error);
	if (!strcmp(vendor, "PCMCIA") && !strcmp(product, "RS-COM 2P"))
		return (puc_bfe_probe(dev, &puc_pccard_rscom));

	return (ENXIO);
}

static device_method_t puc_pccard_methods[] = {
    /* Device interface */
    DEVMETHOD(device_probe,		puc_pccard_probe),
    DEVMETHOD(device_attach,		puc_bfe_attach),
    DEVMETHOD(device_detach,		puc_bfe_detach),

    DEVMETHOD(bus_alloc_resource,	puc_bus_alloc_resource),
    DEVMETHOD(bus_release_resource,	puc_bus_release_resource),
    DEVMETHOD(bus_get_resource,		puc_bus_get_resource),
    DEVMETHOD(bus_read_ivar,		puc_bus_read_ivar),
    DEVMETHOD(bus_setup_intr,		puc_bus_setup_intr),
    DEVMETHOD(bus_teardown_intr,	puc_bus_teardown_intr),
    DEVMETHOD(bus_print_child,		puc_bus_print_child),
    DEVMETHOD(bus_child_pnpinfo_str,	puc_bus_child_pnpinfo_str),
    DEVMETHOD(bus_child_location_str,	puc_bus_child_location_str),

    DEVMETHOD_END
};

static driver_t puc_pccard_driver = {
	puc_driver_name,
	puc_pccard_methods,
	sizeof(struct puc_softc),
};

DRIVER_MODULE(puc, pccard, puc_pccard_driver, puc_devclass, 0, 0);

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
#include <sys/socket.h>

#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <net/if.h>
#include <net/ethernet.h>
#include <net/if_arp.h>

#include <isa/isavar.h>

#include <dev/sbni/if_sbnireg.h>
#include <dev/sbni/if_sbnivar.h>

static int	sbni_probe_isa(device_t);
static int	sbni_attach_isa(device_t);

static device_method_t sbni_isa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,	sbni_probe_isa),
	DEVMETHOD(device_attach, sbni_attach_isa),
	{ 0, 0 }
};

static driver_t sbni_isa_driver = {
	"sbni",
	sbni_isa_methods,
	sizeof(struct sbni_softc)
};

static devclass_t sbni_isa_devclass;
static struct isa_pnp_id  sbni_ids[] = {
	{ 0, NULL }	/* we have no pnp sbni cards atm.  */
};

DRIVER_MODULE(sbni, isa, sbni_isa_driver, sbni_isa_devclass, 0, 0);
MODULE_DEPEND(sbni, isa, 1, 1, 1);

static int
sbni_probe_isa(device_t dev)
{
	struct sbni_softc *sc;
	int error;

	error = ISA_PNP_PROBE(device_get_parent(dev), dev, sbni_ids);
	if (error && error != ENOENT)
		return (error);

	sc = device_get_softc(dev);

 	sc->io_res = bus_alloc_resource(dev, SYS_RES_IOPORT, &sc->io_rid,
					0ul, ~0ul, SBNI_PORTS, RF_ACTIVE);
	if (!sc->io_res) {
		printf("sbni: cannot allocate io ports!\n");
		return (ENOENT);
	}

	if (sbni_probe(sc) != 0) {
		sbni_release_resources(sc);
		return (ENXIO);
	}

	device_set_desc(dev, "Granch SBNI12/ISA adapter");
	return (0);
}


static int
sbni_attach_isa(device_t dev)
{
	struct sbni_softc *sc;
	struct sbni_flags flags;
	int error;
   
	sc = device_get_softc(dev);
	sc->dev = dev;

	sc->irq_res = bus_alloc_resource_any(
	    dev, SYS_RES_IRQ, &sc->irq_rid, RF_ACTIVE);

#ifndef SBNI_DUAL_COMPOUND

	if (sc->irq_res == NULL) {
		device_printf(dev, "irq conflict!\n");
		sbni_release_resources(sc);
		return (ENOENT);
	}

#else	/* SBNI_DUAL_COMPOUND */

	if (sc->irq_res) {
		sbni_add(sc);
	} else {
		struct sbni_softc  *master;

		if ((master = connect_to_master(sc)) == 0) {
			device_printf(dev, "failed to alloc irq\n");
			sbni_release_resources(sc);
			return (ENXIO);
		} else {
			device_printf(dev, "shared irq with %s\n",
			       master->ifp->if_xname);
		}
	} 
#endif	/* SBNI_DUAL_COMPOUND */

	*(u_int32_t*)&flags = device_get_flags(dev);

	error = sbni_attach(sc, device_get_unit(dev) * 2, flags);
	if (error) {
		device_printf(dev, "cannot initialize driver\n");
		sbni_release_resources(sc);
		return (error);
	}

	if (sc->irq_res) {
		error = bus_setup_intr(
		    dev, sc->irq_res, INTR_TYPE_NET | INTR_MPSAFE,
		    NULL, sbni_intr, sc, &sc->irq_handle);
		if (error) {
			device_printf(dev, "bus_setup_intr\n");
			sbni_detach(sc);
			sbni_release_resources(sc);
			return (error);
		}
	}

	return (0);
}
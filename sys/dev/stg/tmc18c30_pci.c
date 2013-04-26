
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
#include <sys/malloc.h>
#include <sys/errno.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

#include <cam/scsi/scsi_low.h>

#include <dev/stg/tmc18c30reg.h>
#include <dev/stg/tmc18c30var.h>
#include <dev/stg/tmc18c30.h>

static struct _pcsid
{
	u_int32_t	type;
	const char	*desc;
} pci_ids[] = {
	{ 0x00001036, 	"Adaptec AHA-2920/A,Future Domain TMC-18XX/3260"	},
	{ 0x00000000, 	NULL							}
};

static int
stg_pci_probe(device_t dev)
{
	u_int32_t type = pci_get_devid(dev);
	struct _pcsid *stg = pci_ids;

	while (stg->type && stg->type != type)
		++stg;
	if (stg->desc) {
		device_set_desc(dev, stg->desc);
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
stg_pci_attach(device_t dev)
{
	struct stg_softc	*sc = device_get_softc(dev);
	int			error;

	sc->port_rid = PCIR_BAR(0);
	sc->irq_rid = 0;
	error = stg_alloc_resource(dev);
	if (error) {
		return(error);
	}

	/* XXXX remove INTR_ENTROPY below for MFC */
	error = bus_setup_intr(dev, sc->irq_res, INTR_TYPE_CAM | INTR_ENTROPY,
			       NULL, stg_intr, (void *)sc, &sc->stg_intrhand);
	if (error) {
		stg_release_resource(dev);
		return(error);
	}

	if (stg_attach(dev) == 0) {
		stg_release_resource(dev);
		return(ENXIO);
	}

	return(0);
}

static device_method_t stg_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		stg_pci_probe),
	DEVMETHOD(device_attach,	stg_pci_attach),
	DEVMETHOD(device_detach,	stg_detach),

	{ 0, 0 }
};

static driver_t stg_pci_driver = {
	"stg",
	stg_pci_methods,
	sizeof(struct stg_softc),
};

DRIVER_MODULE(stg, pci, stg_pci_driver, stg_devclass, 0, 0);
MODULE_DEPEND(stg, scsi_low, 1, 1, 1);
MODULE_DEPEND(stg, pci, 1, 1, 1);
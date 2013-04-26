
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

#include <dev/aic7xxx/aic7xxx_osm.h>

static int ahc_pci_probe(device_t dev);
static int ahc_pci_attach(device_t dev);

static device_method_t ahc_pci_device_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		ahc_pci_probe),
	DEVMETHOD(device_attach,	ahc_pci_attach),
	DEVMETHOD(device_detach,	ahc_detach),
	{ 0, 0 }
};

static driver_t ahc_pci_driver = {
	"ahc",
	ahc_pci_device_methods,
	sizeof(struct ahc_softc)
};

DRIVER_MODULE(ahc_pci, pci, ahc_pci_driver, ahc_devclass, 0, 0);
MODULE_DEPEND(ahc_pci, ahc, 1, 1, 1);
MODULE_VERSION(ahc_pci, 1);

static int
ahc_pci_probe(device_t dev)
{
	struct	ahc_pci_identity *entry;

	entry = ahc_find_pci_device(dev);
	if (entry != NULL) {
		device_set_desc(dev, entry->name);
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}

static int
ahc_pci_attach(device_t dev)
{
	struct	 ahc_pci_identity *entry;
	struct	 ahc_softc *ahc;
	char	*name;
	int	 error;

	entry = ahc_find_pci_device(dev);
	if (entry == NULL)
		return (ENXIO);

	/*
	 * Allocate a softc for this card and
	 * set it up for attachment by our
	 * common detect routine.
	 */
	name = malloc(strlen(device_get_nameunit(dev)) + 1, M_DEVBUF, M_NOWAIT);
	if (name == NULL)
		return (ENOMEM);
	strcpy(name, device_get_nameunit(dev));
	ahc = ahc_alloc(dev, name);
	if (ahc == NULL)
		return (ENOMEM);

	ahc_set_unit(ahc, device_get_unit(dev));

	/*
	 * Should we bother disabling 39Bit addressing
	 * based on installed memory?
	 */
	if (sizeof(bus_addr_t) > 4)
                ahc->flags |= AHC_39BIT_ADDRESSING;

	/* Allocate a dmatag for our SCB DMA maps */
	error = aic_dma_tag_create(ahc, /*parent*/bus_get_dma_tag(dev),
				   /*alignment*/1, /*boundary*/0,
				   (ahc->flags & AHC_39BIT_ADDRESSING)
				   ? 0x7FFFFFFFFFLL
				   : BUS_SPACE_MAXADDR_32BIT,
				   /*highaddr*/BUS_SPACE_MAXADDR,
				   /*filter*/NULL, /*filterarg*/NULL,
				   /*maxsize*/BUS_SPACE_MAXSIZE_32BIT,
				   /*nsegments*/AHC_NSEG,
				   /*maxsegsz*/AHC_MAXTRANSFER_SIZE,
				   /*flags*/0,
				   &ahc->parent_dmat);

	if (error != 0) {
		printf("ahc_pci_attach: Could not allocate DMA tag "
		       "- error %d\n", error);
		ahc_free(ahc);
		return (ENOMEM);
	}
	ahc->dev_softc = dev;
	error = ahc_pci_config(ahc, entry);
	if (error != 0) {
		ahc_free(ahc);
		return (error);
	}

	ahc_attach(ahc);
	return (0);
}

int
ahc_pci_map_registers(struct ahc_softc *ahc)
{
	struct	resource *regs;
	u_int	command;
	int	regs_type;
	int	regs_id;
	int	allow_memio;

	command = aic_pci_read_config(ahc->dev_softc, PCIR_COMMAND, /*bytes*/1);
	regs = NULL;
	regs_type = 0;
	regs_id = 0;

	/* Retrieve the per-device 'allow_memio' hint */
	if (resource_int_value(device_get_name(ahc->dev_softc),
			       device_get_unit(ahc->dev_softc),
			       "allow_memio", &allow_memio) != 0) {
		if (bootverbose)
			device_printf(ahc->dev_softc, "Defaulting to MEMIO ");
#ifdef AHC_ALLOW_MEMIO
		if (bootverbose)
			printf("on\n");
		allow_memio = 1;
#else
		if (bootverbose)
			printf("off\n");
		allow_memio = 0;
#endif
	}

	if ((allow_memio != 0) && (command & PCIM_CMD_MEMEN) != 0) {

		regs_type = SYS_RES_MEMORY;
		regs_id = AHC_PCI_MEMADDR;
		regs = bus_alloc_resource_any(ahc->dev_softc, regs_type,
					      &regs_id, RF_ACTIVE);
		if (regs != NULL) {
			ahc->tag = rman_get_bustag(regs);
			ahc->bsh = rman_get_bushandle(regs);

			/*
			 * Do a quick test to see if memory mapped
			 * I/O is functioning correctly.
			 */
			if (ahc_pci_test_register_access(ahc) != 0) {
				device_printf(ahc->dev_softc,
				       "PCI Device %d:%d:%d failed memory "
				       "mapped test.  Using PIO.\n",
				       aic_get_pci_bus(ahc->dev_softc),
				       aic_get_pci_slot(ahc->dev_softc),
				       aic_get_pci_function(ahc->dev_softc));
				bus_release_resource(ahc->dev_softc, regs_type,
						     regs_id, regs);
				regs = NULL;
			} else {
				command &= ~PCIM_CMD_PORTEN;
				aic_pci_write_config(ahc->dev_softc,
						     PCIR_COMMAND,
						     command, /*bytes*/1);
			}
		}
	}

	if (regs == NULL && (command & PCIM_CMD_PORTEN) != 0) {
		regs_type = SYS_RES_IOPORT;
		regs_id = AHC_PCI_IOADDR;
		regs = bus_alloc_resource_any(ahc->dev_softc, regs_type,
					      &regs_id, RF_ACTIVE);
		if (regs != NULL) {
			ahc->tag = rman_get_bustag(regs);
			ahc->bsh = rman_get_bushandle(regs);
			if (ahc_pci_test_register_access(ahc) != 0) {
				device_printf(ahc->dev_softc,
				       "PCI Device %d:%d:%d failed I/O "
				       "mapped test.\n",
				       aic_get_pci_bus(ahc->dev_softc),
				       aic_get_pci_slot(ahc->dev_softc),
				       aic_get_pci_function(ahc->dev_softc));
				bus_release_resource(ahc->dev_softc, regs_type,
						     regs_id, regs);
				regs = NULL;
			} else {
				command &= ~PCIM_CMD_MEMEN;
				aic_pci_write_config(ahc->dev_softc,
						     PCIR_COMMAND,
						     command, /*bytes*/1);
			}
		}
	}
	if (regs == NULL) {
		device_printf(ahc->dev_softc,
			      "can't allocate register resources\n");
		return (ENOMEM);
	}
	ahc->platform_data->regs_res_type = regs_type;
	ahc->platform_data->regs_res_id = regs_id;
	ahc->platform_data->regs = regs;
	return (0);
}
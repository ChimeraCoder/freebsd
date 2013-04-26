
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

/*
 * Lucent WaveLAN/IEEE 802.11 PCMCIA driver for FreeBSD.
 *
 * Written by Bill Paul <wpaul@ctr.columbia.edu>
 * Electrical Engineering Department
 * Columbia University, New York City
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/systm.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <net/if_media.h>
#include <net/if_types.h>

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_radiotap.h>

#include <dev/wi/if_wavelan_ieee.h>
#include <dev/wi/if_wireg.h>
#include <dev/wi/if_wivar.h>

static int wi_pci_probe(device_t);
static int wi_pci_attach(device_t);
static int wi_pci_suspend(device_t);
static int wi_pci_resume(device_t);

static device_method_t wi_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		wi_pci_probe),
	DEVMETHOD(device_attach,	wi_pci_attach),
	DEVMETHOD(device_detach,	wi_detach),
	DEVMETHOD(device_shutdown,	wi_shutdown),
	DEVMETHOD(device_suspend,	wi_pci_suspend),
	DEVMETHOD(device_resume,	wi_pci_resume),

	{ 0, 0 }
};

static driver_t wi_pci_driver = {
	"wi",
	wi_pci_methods,
	sizeof(struct wi_softc)
};

static struct {
	unsigned int vendor,device;
	int bus_type;
	char *desc;
} pci_ids[] = {
	/* Sorted by description */
	{0x10b7, 0x7770, WI_BUS_PCI_PLX, "3Com Airconnect"},
	{0x16ab, 0x1101, WI_BUS_PCI_PLX, "GLPRISM2 WaveLAN"},
	{0x1260, 0x3872, WI_BUS_PCI_NATIVE, "Intersil Prism3"},
	{0x1260, 0x3873, WI_BUS_PCI_NATIVE, "Intersil Prism2.5"},
	{0x16ab, 0x1102, WI_BUS_PCI_PLX, "Linksys WDT11"},
	{0x1385, 0x4100, WI_BUS_PCI_PLX, "Netgear MA301"},
	{0x1638, 0x1100, WI_BUS_PCI_PLX, "PRISM2STA WaveLAN"},
	{0x111a, 0x1023, WI_BUS_PCI_PLX, "Siemens SpeedStream"},
	{0x10b5, 0x9050, WI_BUS_PCI_PLX, "SMC 2602W"},
	{0x16ec, 0x3685, WI_BUS_PCI_PLX, "US Robotics 2415"},
	{0x4033, 0x7001, WI_BUS_PCI_PLX, "Addtron AWA-100 PCI"},
	{0, 0, 0, NULL}
};

DRIVER_MODULE(wi, pci, wi_pci_driver, wi_devclass, 0, 0);
MODULE_DEPEND(wi, pci, 1, 1, 1);
MODULE_DEPEND(wi, wlan, 1, 1, 1);

static int
wi_pci_probe(dev)
	device_t	dev;
{
	struct wi_softc		*sc;
	int i;

	sc = device_get_softc(dev);
	for(i=0; pci_ids[i].vendor != 0; i++) {
		if ((pci_get_vendor(dev) == pci_ids[i].vendor) &&
			(pci_get_device(dev) == pci_ids[i].device)) {
			sc->wi_bus_type = pci_ids[i].bus_type;
			device_set_desc(dev, pci_ids[i].desc);
			return (BUS_PROBE_DEFAULT);
		}
	}
	return(ENXIO);
}

static int
wi_pci_attach(device_t dev)
{
	struct wi_softc		*sc;
	u_int32_t		command, wanted;
	u_int16_t		reg;
	int			error;
	int			timeout;

	sc = device_get_softc(dev);

	command = pci_read_config(dev, PCIR_COMMAND, 4);
	wanted = PCIM_CMD_PORTEN|PCIM_CMD_MEMEN;
	command |= wanted;
	pci_write_config(dev, PCIR_COMMAND, command, 4);
	command = pci_read_config(dev, PCIR_COMMAND, 4);
	if ((command & wanted) != wanted) {
		device_printf(dev, "wi_pci_attach() failed to enable pci!\n");
		return (ENXIO);
	}

	if (sc->wi_bus_type != WI_BUS_PCI_NATIVE) {
		error = wi_alloc(dev, WI_PCI_IORES);
		if (error)
			return (error);

		/* Make sure interrupts are disabled. */
		CSR_WRITE_2(sc, WI_INT_EN, 0);
		CSR_WRITE_2(sc, WI_EVENT_ACK, 0xFFFF);

		/* We have to do a magic PLX poke to enable interrupts */
		sc->local_rid = WI_PCI_LOCALRES;
		sc->local = bus_alloc_resource_any(dev, SYS_RES_IOPORT,
		    &sc->local_rid, RF_ACTIVE);
		sc->wi_localtag = rman_get_bustag(sc->local);
		sc->wi_localhandle = rman_get_bushandle(sc->local);
		command = bus_space_read_4(sc->wi_localtag, sc->wi_localhandle,
		    WI_LOCAL_INTCSR);
		command |= WI_LOCAL_INTEN;
		bus_space_write_4(sc->wi_localtag, sc->wi_localhandle,
		    WI_LOCAL_INTCSR, command);
		bus_release_resource(dev, SYS_RES_IOPORT, sc->local_rid,
		    sc->local);
		sc->local = NULL;

		sc->mem_rid = WI_PCI_MEMRES;
		sc->mem = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
					&sc->mem_rid, RF_ACTIVE);
		if (sc->mem == NULL) {
			device_printf(dev, "couldn't allocate memory\n");
			wi_free(dev);
			return (ENXIO);
		}
		sc->wi_bmemtag = rman_get_bustag(sc->mem);
		sc->wi_bmemhandle = rman_get_bushandle(sc->mem);

		/*
		 * Write COR to enable PC card
		 * This is a subset of the protocol that the pccard bus code
		 * would do.  In theory, we should parse the CIS to find the
		 * COR offset.  In practice, the COR_OFFSET is always 0x3e0.
		 */
		CSM_WRITE_1(sc, WI_COR_OFFSET, WI_COR_VALUE); 
		reg = CSM_READ_1(sc, WI_COR_OFFSET);
		if (reg != WI_COR_VALUE) {
			device_printf(dev, "CSM_READ_1(WI_COR_OFFSET) "
			    "wanted %d, got %d\n", WI_COR_VALUE, reg);
			wi_free(dev);
			return (ENXIO);
		}
	} else {
		error = wi_alloc(dev, WI_PCI_LMEMRES);
		if (error)
			return (error);

		CSR_WRITE_2(sc, WI_PCICOR_OFF, WI_PCICOR_RESET);
		DELAY(250000);

		CSR_WRITE_2(sc, WI_PCICOR_OFF, 0x0000);
		DELAY(500000);

		timeout=2000000;
		while ((--timeout > 0) &&
		    (CSR_READ_2(sc, WI_COMMAND) & WI_CMD_BUSY))
			DELAY(10);

		if (timeout == 0) {
			device_printf(dev, "couldn't reset prism pci core.\n");
			wi_free(dev);
			return(ENXIO);
		}
	}

	CSR_WRITE_2(sc, WI_HFA384X_SWSUPPORT0_OFF, WI_PRISM2STA_MAGIC);
	reg = CSR_READ_2(sc, WI_HFA384X_SWSUPPORT0_OFF);
	if (reg != WI_PRISM2STA_MAGIC) {
		device_printf(dev,
		    "CSR_READ_2(WI_HFA384X_SWSUPPORT0_OFF) "
		    "wanted %d, got %d\n", WI_PRISM2STA_MAGIC, reg);
		wi_free(dev);
		return (ENXIO);
	}

	error = wi_attach(dev);
	if (error != 0)
		wi_free(dev);
	return (error);
}

static int
wi_pci_suspend(device_t dev)
{
	struct wi_softc	*sc = device_get_softc(dev);

	wi_stop(sc, 1);
	
	return (0);
}

static int
wi_pci_resume(device_t dev)
{
	struct wi_softc	*sc = device_get_softc(dev);
	struct ifnet *ifp = sc->sc_ifp;

	if (sc->wi_bus_type != WI_BUS_PCI_NATIVE)
		return (0);

	if (ifp->if_flags & IFF_UP) {
		ifp->if_init(ifp->if_softc);
		if (ifp->if_drv_flags & IFF_DRV_RUNNING)
			ifp->if_start(ifp);
	}

	return (0);
}
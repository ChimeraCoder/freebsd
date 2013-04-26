
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
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/module.h>
#include <sys/tty.h>
#include <machine/bus.h>
#include <sys/timepps.h>

#include <dev/sio/siovar.h>

#include <dev/pci/pcivar.h>

static	int	sio_pci_attach(device_t dev);
static	int	sio_pci_probe(device_t dev);

static device_method_t sio_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		sio_pci_probe),
	DEVMETHOD(device_attach,	sio_pci_attach),
	DEVMETHOD(device_detach,	siodetach),

	{ 0, 0 }
};

static driver_t sio_pci_driver = {
	sio_driver_name,
	sio_pci_methods,
	0,
};

struct pci_ids {
	u_int32_t	type;
	const char	*desc;
	int		rid;
};

static struct pci_ids pci_ids[] = {
	{ 0x100812b9, "3COM PCI FaxModem", 0x10 },
	{ 0x2000131f, "CyberSerial (1-port) 16550", 0x10 },
	{ 0x01101407, "Koutech IOFLEX-2S PCI Dual Port Serial", 0x10 },
	{ 0x01111407, "Koutech IOFLEX-2S PCI Dual Port Serial", 0x10 },
	{ 0x048011c1, "Lucent kermit based PCI Modem", 0x14 },
	{ 0x95211415, "Oxford Semiconductor PCI Dual Port Serial", 0x10 },
	{ 0x7101135e, "SeaLevel Ultra 530.PCI Single Port Serial", 0x18 },
	{ 0x0000151f, "SmartLink 5634PCV SurfRider", 0x10 },
	{ 0x0103115d, "Xircom Cardbus modem", 0x10 },
	{ 0x432214e4, "Broadcom 802.11b/GPRS CardBus (Serial)", 0x10 },
	{ 0x434414e4, "Broadcom 802.11bg/EDGE/GPRS CardBus (Serial)", 0x10 },
	{ 0x01c0135c, "Quatech SSCLP-200/300", 0x18 
		/* 
		 * NB: You must mount the "SPAD" jumper to correctly detect
		 * the FIFO on the UART.  Set the options on the jumpers,
		 * we do not support the extra registers on the Quatech.
		 */
	},
	{ 0x00000000, NULL, 0 }
};

static int
sio_pci_attach(dev)
	device_t	dev;
{
	u_int32_t	type;
	struct pci_ids	*id;

	type = pci_get_devid(dev);
	id = pci_ids;
	while (id->type && id->type != type)
		id++;
	if (id->desc == NULL)
		return (ENXIO);
	return (sioattach(dev, id->rid, 0UL));
}

static int
sio_pci_probe(dev)
	device_t	dev;
{
	u_int32_t	type;
	struct pci_ids	*id;

	type = pci_get_devid(dev);
	id = pci_ids;
	while (id->type && id->type != type)
		id++;
	if (id->desc == NULL)
		return (ENXIO);
	device_set_desc(dev, id->desc);
#ifdef PC98
	SET_FLAG(dev, SET_IFTYPE(COM_IF_NS16550));
#endif
	return (sioprobe(dev, id->rid, 0UL, 0));
}

DRIVER_MODULE(sio, pci, sio_pci_driver, sio_devclass, 0, 0);
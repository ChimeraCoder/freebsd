
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

#include "opt_ar71xx.h"

#include <sys/param.h>
#include <sys/systm.h>

#include <sys/bus.h>
#include <sys/interrupt.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/intr_machdep.h>
#include <machine/pmap.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#include <dev/pci/pcib_private.h>
#include "pcib_if.h"

#include <mips/atheros/ar71xxreg.h>
#include <mips/atheros/ar71xx_pci_bus_space.h>

#include <mips/atheros/ar71xx_cpudef.h>

#include <sys/linker.h>
#include <sys/firmware.h>

#include <mips/atheros/ar71xx_fixup.h>

/*
 * Take a copy of the EEPROM contents and squirrel it away in a firmware.
 * The SPI flash will eventually cease to be memory-mapped, so we need
 * to take a copy of this before the SPI driver initialises.
 */
void
ar71xx_pci_slot_create_eeprom_firmware(device_t dev, u_int bus, u_int slot,
    u_int func, long int flash_addr, int size)
{
	char buf[64];
	uint16_t *cal_data = (uint16_t *) MIPS_PHYS_TO_KSEG1(flash_addr);
	void *eeprom = NULL;
	const struct firmware *fw = NULL;

	device_printf(dev, "EEPROM firmware: 0x%lx @ %d bytes\n",
	    flash_addr, size);

	eeprom = malloc(size, M_DEVBUF, M_WAITOK | M_ZERO);
	if (! eeprom) {
		device_printf(dev,
			    "%s: malloc failed for '%s', aborting EEPROM\n",
			    __func__, buf);
			return;
	}

	memcpy(eeprom, cal_data, size);

	/*
	 * Generate a flash EEPROM 'firmware' from the given memory
	 * region.  Since the SPI controller will eventually
	 * go into port-IO mode instead of memory-mapped IO
	 * mode, a copy of the EEPROM contents is required.
	 */
	snprintf(buf, sizeof(buf), "%s.%d.bus.%d.%d.%d.eeprom_firmware",
	    device_get_name(dev), device_get_unit(dev), bus, slot, func);
	fw = firmware_register(buf, eeprom, size, 1, NULL);
	if (fw == NULL) {
		device_printf(dev, "%s: firmware_register (%s) failed\n",
		    __func__, buf);
		free(eeprom, M_DEVBUF);
		return;
	}
	device_printf(dev, "device EEPROM '%s' registered\n", buf);
}

#if 0
static void
ar71xx_pci_slot_fixup(device_t dev, u_int bus, u_int slot, u_int func)
{
	long int flash_addr;
	char buf[64];
	int size;

	/*
	 * Check whether the given slot has a hint to poke.
	 */
	if (bootverbose)
	device_printf(dev, "%s: checking dev %s, %d/%d/%d\n",
	    __func__, device_get_nameunit(dev), bus, slot, func);

	snprintf(buf, sizeof(buf), "bus.%d.%d.%d.ath_fixup_addr",
	    bus, slot, func);

	if (resource_long_value(device_get_name(dev), device_get_unit(dev),
	    buf, &flash_addr) == 0) {
		snprintf(buf, sizeof(buf), "bus.%d.%d.%d.ath_fixup_size",
		    bus, slot, func);
		if (resource_int_value(device_get_name(dev),
		    device_get_unit(dev), buf, &size) != 0) {
			device_printf(dev,
			    "%s: missing hint '%s', aborting EEPROM\n",
			    __func__, buf);
			return;
		}


		device_printf(dev, "found EEPROM at 0x%lx on %d.%d.%d\n",
		    flash_addr, bus, slot, func);
		ar71xx_pci_fixup(dev, bus, slot, func, flash_addr, size);
		ar71xx_pci_slot_create_eeprom_firmware(dev, bus, slot, func,
		    flash_addr, size);
	}
}
#endif /* 0 */
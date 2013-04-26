
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
 * 6.7 : Hardware Abstraction
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <contrib/dev/acpica/include/acpi.h>

#include <machine/iodev.h>
#include <machine/pci_cfgreg.h>

/*
 * ACPICA's rather gung-ho approach to hardware resource ownership is a little
 * troublesome insofar as there is no easy way for us to know in advance
 * exactly which I/O resources it's going to want to use.
 *
 * In order to deal with this, we ignore resource ownership entirely, and simply
 * use the native I/O space accessor functionality.  This is Evil, but it works.
 */

ACPI_STATUS
AcpiOsReadPort(ACPI_IO_ADDRESS InPort, UINT32 *Value, UINT32 Width)
{

    switch (Width) {
    case 8:
	*Value = iodev_read_1(InPort);
	break;
    case 16:
	*Value = iodev_read_2(InPort);
	break;
    case 32:
	*Value = iodev_read_4(InPort);
	break;
    }

    return (AE_OK);
}

ACPI_STATUS
AcpiOsWritePort(ACPI_IO_ADDRESS OutPort, UINT32	Value, UINT32 Width)
{

    switch (Width) {
    case 8:
	iodev_write_1(OutPort, Value);
	break;
    case 16:
	iodev_write_2(OutPort, Value);
	break;
    case 32:
	iodev_write_4(OutPort, Value);
	break;
    }

    return (AE_OK);
}

ACPI_STATUS
AcpiOsReadPciConfiguration(ACPI_PCI_ID *PciId, UINT32 Register, UINT64 *Value,
    UINT32 Width)
{

    if (Width == 64)
	return (AE_SUPPORT);

    if (!pci_cfgregopen())
	return (AE_NOT_EXIST);

    *(UINT64 *)Value = pci_cfgregread(PciId->Bus, PciId->Device,
	PciId->Function, Register, Width / 8);

    return (AE_OK);
}


ACPI_STATUS
AcpiOsWritePciConfiguration (ACPI_PCI_ID *PciId, UINT32 Register,
    UINT64 Value, UINT32 Width)
{

    if (Width == 64)
	return (AE_SUPPORT);

    if (!pci_cfgregopen())
    	return (AE_NOT_EXIST);

    pci_cfgregwrite(PciId->Bus, PciId->Device, PciId->Function, Register,
	Value, Width / 8);

    return (AE_OK);
}
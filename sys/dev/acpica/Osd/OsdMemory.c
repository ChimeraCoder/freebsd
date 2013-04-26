
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
 * 6.2 : Memory Management
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <contrib/dev/acpica/include/acpi.h>

#include <sys/kernel.h>
#include <sys/malloc.h>
#include <vm/vm.h>
#include <vm/pmap.h>

static MALLOC_DEFINE(M_ACPICA, "acpica", "ACPI CA memory pool");

void *
AcpiOsAllocate(ACPI_SIZE Size)
{
    return (malloc(Size, M_ACPICA, M_NOWAIT));
}

void
AcpiOsFree(void *Memory)
{
    free(Memory, M_ACPICA);
}

void *
AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
    return (pmap_mapbios((vm_offset_t)PhysicalAddress, Length));
}

void
AcpiOsUnmapMemory(void *LogicalAddress, ACPI_SIZE Length)
{
    pmap_unmapbios((vm_offset_t)LogicalAddress, Length);
}

ACPI_STATUS
AcpiOsGetPhysicalAddress(void *LogicalAddress,
    ACPI_PHYSICAL_ADDRESS *PhysicalAddress)
{
    /* We can't necessarily do this, so cop out. */
    return (AE_BAD_ADDRESS);
}

BOOLEAN
AcpiOsReadable (void *Pointer, ACPI_SIZE Length)
{
    return (TRUE);
}

BOOLEAN
AcpiOsWritable (void *Pointer, ACPI_SIZE Length)
{
    return (TRUE);
}

ACPI_STATUS
AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width)
{
    void	*LogicalAddress;

    LogicalAddress = pmap_mapdev(Address, Width / 8);
    if (LogicalAddress == NULL)
	return (AE_NOT_EXIST);

    switch (Width) {
    case 8:
	*Value = *(volatile uint8_t *)LogicalAddress;
	break;
    case 16:
	*Value = *(volatile uint16_t *)LogicalAddress;
	break;
    case 32:
	*Value = *(volatile uint32_t *)LogicalAddress;
	break;
    case 64:
	*Value = *(volatile uint64_t *)LogicalAddress;
	break;
    }

    pmap_unmapdev((vm_offset_t)LogicalAddress, Width / 8);

    return (AE_OK);
}

ACPI_STATUS
AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
    void	*LogicalAddress;

    LogicalAddress = pmap_mapdev(Address, Width / 8);
    if (LogicalAddress == NULL)
	return (AE_NOT_EXIST);

    switch (Width) {
    case 8:
	*(volatile uint8_t *)LogicalAddress = Value;
	break;
    case 16:
	*(volatile uint16_t *)LogicalAddress = Value;
	break;
    case 32:
	*(volatile uint32_t *)LogicalAddress = Value;
	break;
    case 64:
	*(volatile uint64_t *)LogicalAddress = Value;
	break;
    }

    pmap_unmapdev((vm_offset_t)LogicalAddress, Width / 8);

    return (AE_OK);
}
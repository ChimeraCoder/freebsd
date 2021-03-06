
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

#include <sys/types.h>
#include <sys/bus.h>
#include <sys/sysctl.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/actables.h>

static u_long acpi_root_phys;

SYSCTL_ULONG(_machdep, OID_AUTO, acpi_root, CTLFLAG_RD, &acpi_root_phys, 0,
    "The physical address of the RSDP");

ACPI_STATUS
AcpiOsInitialize(void)
{

	return (AE_OK);
}

ACPI_STATUS
AcpiOsTerminate(void)
{

	return (AE_OK);
}

static u_long
acpi_get_root_from_loader(void)
{
	long acpi_root;

	if (resource_long_value("acpi", 0, "rsdp", &acpi_root) == 0)
		return (acpi_root);

	return (0);
}

static u_long
acpi_get_root_from_memory(void)
{
	ACPI_SIZE acpi_root;

	if (ACPI_SUCCESS(AcpiFindRootPointer(&acpi_root)))
		return (acpi_root);

	return (0);
}

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(void)
{

	if (acpi_root_phys == 0) {
		acpi_root_phys = acpi_get_root_from_loader();
		if (acpi_root_phys == 0)
			acpi_root_phys = acpi_get_root_from_memory();
	}

	return (acpi_root_phys);
}
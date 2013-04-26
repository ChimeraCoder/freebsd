
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
 * ACPI Table interfaces
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/endian.h>
#include <sys/kernel.h>
#include <sys/linker.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/actables.h>

#undef _COMPONENT
#define	_COMPONENT      ACPI_TABLES

static char acpi_osname[128];
TUNABLE_STR("hw.acpi.osname", acpi_osname, sizeof(acpi_osname));

ACPI_STATUS
AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES *InitVal,
    ACPI_STRING *NewVal)
{

    if (InitVal == NULL || NewVal == NULL)
	return (AE_BAD_PARAMETER);

    *NewVal = NULL;
    if (strncmp(InitVal->Name, "_OS_", ACPI_NAME_SIZE) == 0 &&
	strlen(acpi_osname) > 0) {
	printf("ACPI: Overriding _OS definition with \"%s\"\n", acpi_osname);
	*NewVal = acpi_osname;
    }

    return (AE_OK);
}

ACPI_STATUS
AcpiOsTableOverride(ACPI_TABLE_HEADER *ExistingTable,
    ACPI_TABLE_HEADER **NewTable)
{
    char modname[] = "acpi_dsdt";
    caddr_t acpi_table;
    ACPI_TABLE_HEADER *hdr;
    size_t sz;

    if (ExistingTable == NULL || NewTable == NULL)
	return (AE_BAD_PARAMETER);

    *NewTable = NULL;
#ifdef notyet
    for (int i = 0; i < ACPI_NAME_SIZE; i++)
	modname[i + 5] = tolower(ExistingTable->Signature[i]);
#else
    /* If we're not overriding the DSDT, just return. */
    if (strncmp(ExistingTable->Signature, ACPI_SIG_DSDT, ACPI_NAME_SIZE) != 0)
	return (AE_OK);
#endif

    acpi_table = preload_search_by_type(modname);
    if (acpi_table == NULL)
	return (AE_OK);

    hdr = preload_fetch_addr(acpi_table);
    sz = preload_fetch_size(acpi_table);
    if (hdr != NULL && sz != 0)
	*NewTable = hdr;

    return (AE_OK);
}

ACPI_STATUS
AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER *ExistingTable,
    ACPI_PHYSICAL_ADDRESS *NewAddress, UINT32 *NewTableLength)
{

	return (AE_SUPPORT);
}

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

#include <sys/param.h>
#include <sys/bus.h>
#include <machine/md_var.h>
#include <machine/pal.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/actables.h>
#include <dev/acpica/acpivar.h>

int
acpi_machdep_init(device_t dev)
{
	struct	acpi_softc *sc;

        sc = device_get_softc(dev);

	acpi_install_wakeup_handler(sc);

	return (0);
}

int
acpi_machdep_quirks(int *quirks)
{
	return (0);
}

void
acpi_cpu_c1()
{
#ifdef INVARIANTS
	register_t ie;

	ie = intr_disable();
	KASSERT(ie == 0, ("%s called with interrupts enabled\n", __func__));
#endif
	ia64_call_pal_static(PAL_HALT_LIGHT, 0, 0, 0);
	ia64_enable_intr();
}

void *
acpi_find_table(const char *sig)
{
	ACPI_PHYSICAL_ADDRESS rsdp_ptr;
	ACPI_TABLE_RSDP *rsdp;
	ACPI_TABLE_XSDT *xsdt;
	ACPI_TABLE_HEADER *table;
	UINT64 addr;
	u_int i, count;

	if ((rsdp_ptr = AcpiOsGetRootPointer()) == 0)
		return (NULL);

	rsdp = (ACPI_TABLE_RSDP *)IA64_PHYS_TO_RR7(rsdp_ptr);
	xsdt = (ACPI_TABLE_XSDT *)IA64_PHYS_TO_RR7(rsdp->XsdtPhysicalAddress);

	count = (UINT64 *)((char *)xsdt + xsdt->Header.Length) -
	    xsdt->TableOffsetEntry;

	for (i = 0; i < count; i++) {
		addr = xsdt->TableOffsetEntry[i];
		table = (ACPI_TABLE_HEADER *)IA64_PHYS_TO_RR7(addr);

		if (strncmp(table->Signature, sig, ACPI_NAME_SIZE) != 0)
			continue;
		if (ACPI_FAILURE(AcpiTbChecksum((void *)table, table->Length)))
			continue;

		return (table);
	}

	return (NULL);
}
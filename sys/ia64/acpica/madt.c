
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

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/actables.h>

#include <machine/md_var.h>

struct sapic *sapic_create(int, int, u_int64_t);

static void
print_entry(ACPI_SUBTABLE_HEADER *entry)
{

	switch (entry->Type) {
	case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE: {
		ACPI_MADT_INTERRUPT_OVERRIDE *iso =
		    (ACPI_MADT_INTERRUPT_OVERRIDE *)entry;
		printf("\tInterrupt source override entry\n");
		printf("\t\tBus=%u, Source=%u, Irq=0x%x\n", iso->Bus,
		    iso->SourceIrq, iso->GlobalIrq);
		break;
	}

	case ACPI_MADT_TYPE_IO_APIC:
		printf("\tI/O APIC entry\n");
		break;

	case ACPI_MADT_TYPE_IO_SAPIC: {
		ACPI_MADT_IO_SAPIC *sapic = (ACPI_MADT_IO_SAPIC *)entry;
		printf("\tI/O SAPIC entry\n");
		printf("\t\tId=0x%x, InterruptBase=0x%x, Address=0x%lx\n",
		    sapic->Id, sapic->GlobalIrqBase, sapic->Address);
		break;
	}

	case ACPI_MADT_TYPE_LOCAL_APIC_NMI:
		printf("\tLocal APIC NMI entry\n");
		break;

	case ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE: {
		ACPI_MADT_LOCAL_APIC_OVERRIDE *lapic =
		    (ACPI_MADT_LOCAL_APIC_OVERRIDE *)entry;
		printf("\tLocal APIC override entry\n");
		printf("\t\tLocal APIC address=0x%jx\n", lapic->Address);
		break;
	}

	case ACPI_MADT_TYPE_LOCAL_SAPIC: {
		ACPI_MADT_LOCAL_SAPIC *sapic = (ACPI_MADT_LOCAL_SAPIC *)entry;
		printf("\tLocal SAPIC entry\n");
		printf("\t\tProcessorId=0x%x, Id=0x%x, Eid=0x%x",
		    sapic->ProcessorId, sapic->Id, sapic->Eid);
		if (!(sapic->LapicFlags & ACPI_MADT_ENABLED))
			printf(" (disabled)");
		printf("\n");
		break;
	}

	case ACPI_MADT_TYPE_NMI_SOURCE:
		printf("\tNMI entry\n");
		break;

	case ACPI_MADT_TYPE_INTERRUPT_SOURCE: {
		ACPI_MADT_INTERRUPT_SOURCE *pis =
		    (ACPI_MADT_INTERRUPT_SOURCE *)entry;
		printf("\tPlatform interrupt entry\n");
		printf("\t\tPolarity=%u, TriggerMode=%u, Id=0x%x, "
		    "Eid=0x%x, Vector=0x%x, Irq=%d\n",
		    pis->IntiFlags & ACPI_MADT_POLARITY_MASK,
		    (pis->IntiFlags & ACPI_MADT_TRIGGER_MASK) >> 2,
		    pis->Id, pis->Eid, pis->IoSapicVector, pis->GlobalIrq);
		break;
	}

	case ACPI_MADT_TYPE_LOCAL_APIC:
		printf("\tLocal APIC entry\n");
		break;

	default:
		printf("\tUnknown type %d entry\n", entry->Type);
		break;
	}
}

void
ia64_probe_sapics(void)
{
	ACPI_PHYSICAL_ADDRESS rsdp_ptr;
	ACPI_SUBTABLE_HEADER *entry;
	ACPI_TABLE_MADT *table;
	ACPI_TABLE_RSDP *rsdp;
	ACPI_TABLE_XSDT *xsdt;
	char *end, *p;
	int t, tables;

	if ((rsdp_ptr = AcpiOsGetRootPointer()) == 0)
		return;

	rsdp = (ACPI_TABLE_RSDP *)IA64_PHYS_TO_RR7(rsdp_ptr);
	xsdt = (ACPI_TABLE_XSDT *)IA64_PHYS_TO_RR7(rsdp->XsdtPhysicalAddress);

	tables = (UINT64 *)((char *)xsdt + xsdt->Header.Length) -
	    xsdt->TableOffsetEntry;

	for (t = 0; t < tables; t++) {
		table = (ACPI_TABLE_MADT *)
		    IA64_PHYS_TO_RR7(xsdt->TableOffsetEntry[t]);

		if (bootverbose)
			printf("Table '%c%c%c%c' at %p\n",
			    table->Header.Signature[0],
			    table->Header.Signature[1],
			    table->Header.Signature[2],
			    table->Header.Signature[3], table);

		if (strncmp(table->Header.Signature, ACPI_SIG_MADT,
		    ACPI_NAME_SIZE) != 0 ||
		    ACPI_FAILURE(AcpiTbChecksum((void *)table,
		    table->Header.Length)))
			continue;

		/* Save the address of the processor interrupt block. */
		if (bootverbose)
			printf("\tLocal APIC address=0x%x\n", table->Address);
		ia64_lapic_addr = table->Address;

		end = (char *)table + table->Header.Length;
		p = (char *)(table + 1);
		while (p < end) {
			entry = (ACPI_SUBTABLE_HEADER *)p;

			if (bootverbose)
				print_entry(entry);

			switch (entry->Type) {
			case ACPI_MADT_TYPE_IO_SAPIC: {
				ACPI_MADT_IO_SAPIC *sapic =
				    (ACPI_MADT_IO_SAPIC *)entry;
				sapic_create(sapic->Id, sapic->GlobalIrqBase,
				    sapic->Address);
				break;
			}

			case ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE: {
				ACPI_MADT_LOCAL_APIC_OVERRIDE *lapic =
				    (ACPI_MADT_LOCAL_APIC_OVERRIDE *)entry;
				ia64_lapic_addr = lapic->Address;
				break;
			}

#ifdef SMP
			case ACPI_MADT_TYPE_LOCAL_SAPIC: {
				ACPI_MADT_LOCAL_SAPIC *sapic =
				    (ACPI_MADT_LOCAL_SAPIC *)entry;
				if (sapic->LapicFlags & ACPI_MADT_ENABLED)
					cpu_mp_add(sapic->ProcessorId,
					    sapic->Id, sapic->Eid);
				break;
			}
#endif

			default:
				break;
			}

			p += entry->Length;
		}
	}
}

/*
 * Count the number of local SAPIC entries in the APIC table. Every enabled
 * entry corresponds to a processor.
 */
int
ia64_count_cpus(void)
{
	ACPI_PHYSICAL_ADDRESS rsdp_ptr;
	ACPI_MADT_LOCAL_SAPIC *entry;
	ACPI_TABLE_MADT *table;
	ACPI_TABLE_RSDP *rsdp;
	ACPI_TABLE_XSDT *xsdt;
	char *end, *p;
	int cpus, t, tables;

	if ((rsdp_ptr = AcpiOsGetRootPointer()) == 0)
		return (0);

	rsdp = (ACPI_TABLE_RSDP *)IA64_PHYS_TO_RR7(rsdp_ptr);
	xsdt = (ACPI_TABLE_XSDT *)IA64_PHYS_TO_RR7(rsdp->XsdtPhysicalAddress);

	tables = (UINT64 *)((char *)xsdt + xsdt->Header.Length) -
	    xsdt->TableOffsetEntry;

	cpus = 0;

	for (t = 0; t < tables; t++) {
		table = (ACPI_TABLE_MADT *)
		    IA64_PHYS_TO_RR7(xsdt->TableOffsetEntry[t]);

		if (strncmp(table->Header.Signature, ACPI_SIG_MADT,
		    ACPI_NAME_SIZE) != 0 ||
		    ACPI_FAILURE(AcpiTbChecksum((void *)table,
			table->Header.Length)))
			continue;

		end = (char *)table + table->Header.Length;
		p = (char *)(table + 1);
		while (p < end) {
			entry = (ACPI_MADT_LOCAL_SAPIC *)p;

			if (entry->Header.Type == ACPI_MADT_TYPE_LOCAL_SAPIC &&
			    (entry->LapicFlags & ACPI_MADT_ENABLED))
				cpus++;

			p += entry->Header.Length;
		}
	}

	return (cpus);
}

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

#include <stand.h>
#include <machine/stdarg.h>
#include <bootstrap.h>
#include <btxv86.h>
#include "libi386.h"

#include "platform/acfreebsd.h"
#include "acconfig.h"
#define ACPI_SYSTEM_XFACE
#include "actypes.h"
#include "actbl.h"

/*
 * Detect ACPI and export information about the ACPI BIOS into the
 * environment.
 */

static ACPI_TABLE_RSDP	*biosacpi_find_rsdp(void);
static ACPI_TABLE_RSDP	*biosacpi_search_rsdp(char *base, int length);

#define RSDP_CHECKSUM_LENGTH 20

void
biosacpi_detect(void)
{
    ACPI_TABLE_RSDP	*rsdp;
    char		buf[24];
    int			revision;

    /* locate and validate the RSDP */
    if ((rsdp = biosacpi_find_rsdp()) == NULL)
	return;

    /* export values from the RSDP */
    sprintf(buf, "%u", VTOP(rsdp));
    setenv("hint.acpi.0.rsdp", buf, 1);
    revision = rsdp->Revision;
    if (revision == 0)
	revision = 1;
    sprintf(buf, "%d", revision);
    setenv("hint.acpi.0.revision", buf, 1);
    strncpy(buf, rsdp->OemId, sizeof(rsdp->OemId));
    buf[sizeof(rsdp->OemId)] = '\0';
    setenv("hint.acpi.0.oem", buf, 1);
    sprintf(buf, "0x%08x", rsdp->RsdtPhysicalAddress);
    setenv("hint.acpi.0.rsdt", buf, 1);
    if (revision >= 2) {
	/* XXX extended checksum? */
	sprintf(buf, "0x%016llx", rsdp->XsdtPhysicalAddress);
	setenv("hint.acpi.0.xsdt", buf, 1);
	sprintf(buf, "%d", rsdp->Length);
	setenv("hint.acpi.0.xsdt_length", buf, 1);
    }
}

/*
 * Find the RSDP in low memory.  See section 5.2.2 of the ACPI spec.
 */
static ACPI_TABLE_RSDP *
biosacpi_find_rsdp(void)
{
    ACPI_TABLE_RSDP	*rsdp;
    uint16_t		*addr;

    /* EBDA is the 1 KB addressed by the 16 bit pointer at 0x40E. */
    addr = (uint16_t *)PTOV(0x40E);
    if ((rsdp = biosacpi_search_rsdp((char *)(*addr << 4), 0x400)) != NULL)
	return (rsdp);

    /* Check the upper memory BIOS space, 0xe0000 - 0xfffff. */
    if ((rsdp = biosacpi_search_rsdp((char *)0xe0000, 0x20000)) != NULL)
	return (rsdp);

    return (NULL);
}

static ACPI_TABLE_RSDP *
biosacpi_search_rsdp(char *base, int length)
{
    ACPI_TABLE_RSDP	*rsdp;
    u_int8_t		*cp, sum;
    int			ofs, idx;

    /* search on 16-byte boundaries */
    for (ofs = 0; ofs < length; ofs += 16) {
	rsdp = (ACPI_TABLE_RSDP *)PTOV(base + ofs);

	/* compare signature, validate checksum */
	if (!strncmp(rsdp->Signature, ACPI_SIG_RSDP, strlen(ACPI_SIG_RSDP))) {
	    cp = (u_int8_t *)rsdp;
	    sum = 0;
	    for (idx = 0; idx < RSDP_CHECKSUM_LENGTH; idx++)
		sum += *(cp + idx);
	    if (sum != 0)
		continue;
	    return(rsdp);
	}
    }
    return(NULL);
}
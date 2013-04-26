
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

/*
 * Helper functions which can be used in both ISA and EBus code.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>

#include <dev/ofw/ofw_bus_subr.h>
#include <dev/ofw/openfirm.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <sparc64/pci/ofw_pci.h>
#include <sparc64/isa/ofw_isa.h>

#include "pcib_if.h"

int
ofw_isa_range_restype(struct isa_ranges *range)
{
	int ps = ISA_RANGE_PS(range);

	switch (ps) {
	case OFW_PCI_CS_IO:
		return (SYS_RES_IOPORT);
	case OFW_PCI_CS_MEM32:
		return (SYS_RES_MEMORY);
	default:
		panic("ofw_isa_range_restype: illegal space %x", ps);
	}

}

/* XXX: this only supports PCI as parent bus right now. */
int
ofw_isa_range_map(struct isa_ranges *range, int nrange, u_long *start,
    u_long *end, int *which)
{
	struct isa_ranges *r;
	uint64_t offs, cstart, cend;
	int i;

	for (i = 0; i < nrange; i++) {
		r = &range[i];
		cstart = ISA_RANGE_CHILD(r);
		cend = cstart + r->size - 1;
		if (*start < cstart || *start > cend)
			continue;
		if (*end < cstart || *end > cend) {
			panic("ofw_isa_map_iorange: iorange crosses PCI "
			    "ranges (%#lx not in %#lx - %#lx)", *end, cstart,
			    cend);
		}
		offs = ISA_RANGE_PHYS(r);
		*start = *start + offs - cstart;
		*end  = *end + offs - cstart;
		if (which != NULL)
			*which = i;
		return (ofw_isa_range_restype(r));
	}
	panic("ofw_isa_map_iorange: could not map range %#lx - %#lx",
	    *start, *end);
}

ofw_pci_intr_t
ofw_isa_route_intr(device_t bridge, phandle_t node, struct ofw_bus_iinfo *ii,
    ofw_isa_intr_t intr)
{
	struct isa_regs reg;
	uint8_t maskbuf[sizeof(reg) + sizeof(intr)];
	device_t pbridge;
	ofw_isa_intr_t mintr;

	pbridge = device_get_parent(device_get_parent(bridge));
	/*
	 * If we get a match from using the map, the resulting INO is
	 * fully specified, so we may not continue to map.
	 */
	if (!ofw_bus_lookup_imap(node, ii, &reg, sizeof(reg),
	    &intr, sizeof(intr), &mintr, sizeof(mintr), NULL, maskbuf)) {
		/* Try routing at the parent bridge. */
		mintr = PCIB_ROUTE_INTERRUPT(pbridge, bridge, intr);
	}
	return (mintr);
}
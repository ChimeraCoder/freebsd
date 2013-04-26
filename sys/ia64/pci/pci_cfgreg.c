
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
#include <machine/cpufunc.h>
#include <machine/pci_cfgreg.h>
#include <machine/sal.h>

static u_long
pci_sal_address(int dom, int bus, int slot, int func, int reg)
{
	u_long addr;

	addr = ~0ul;
	if (dom >= 0 && dom <= 255 && bus >= 0 && bus <= 255 &&
	    slot >= 0 && slot <= 31 && func >= 0 && func <= 7 &&
	    reg >= 0 && reg <= 255) {
		addr = ((u_long)dom << 24) | ((u_long)bus << 16) |
		    ((u_long)slot << 11) | ((u_long)func << 8) | (u_long)reg;
	}
	return (addr);
}

static int
pci_valid_access(int reg, int len)
{
	int ok;

	ok = ((len == 1 || len == 2 || len == 4) && (reg & (len - 1)) == 0)
	    ? 1 : 0;
	return (ok);
}

int
pci_cfgregopen(void)
{
	return (1);
}

uint32_t
pci_cfgregread(int bus, int slot, int func, int reg, int len)
{
	struct ia64_sal_result res;
	register_t is;
	u_long addr;

	addr = pci_sal_address(0, bus, slot, func, reg);
	if (addr == ~0ul)
		return (~0);

	if (!pci_valid_access(reg, len))
		return (~0);

	is = intr_disable();
	res = ia64_sal_entry(SAL_PCI_CONFIG_READ, addr, len, 0, 0, 0, 0, 0);
	intr_restore(is);

	return ((res.sal_status < 0) ? ~0 : res.sal_result[0]);
}

void
pci_cfgregwrite(int bus, int slot, int func, int reg, uint32_t data, int len)
{
	struct ia64_sal_result res;
	register_t is;
	u_long addr;

	addr = pci_sal_address(0, bus, slot, func, reg);
	if (addr == ~0ul)
		return;

	if (!pci_valid_access(reg, len))
		return;

	is = intr_disable();
	res = ia64_sal_entry(SAL_PCI_CONFIG_WRITE, addr, len, data, 0, 0, 0, 0);
	intr_restore(is);
}
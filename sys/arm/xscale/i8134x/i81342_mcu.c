
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

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/bus.h>
#include <arm/xscale/i8134x/i81342reg.h>
#include <arm/xscale/i8134x/i81342var.h>

void
i81342_sdram_bounds(bus_space_tag_t bt, bus_space_handle_t bh,
    vm_paddr_t *start, vm_size_t *size)
{
	uint32_t reg;
	int bank_nb;

	reg = bus_space_read_4(bt, bh, SMC_SDBR);
	*start = (reg & SMC_SDBR_BASEADDR_MASK);
	reg = bus_space_read_4(bt, bh, SMC_SBSR);
	if (reg & SMC_SBSR_BANK_NB)
		bank_nb = 1;
	else
		bank_nb = 2;
	
	*size = (reg & SMC_SBSR_BANK_SZ_MASK) * bank_nb;
}

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
#include <machine/ia64_cpu.h>

#include "libia64.h"

void
ia64_sync_icache(vm_offset_t va, size_t sz)
{
	uintptr_t pa;
	size_t cnt, max;

	while (sz > 0) {
		max = sz;
		pa = (uintptr_t)ia64_va2pa(va, &max);
		for (cnt = 0; cnt < max; cnt += 32)
			ia64_fc_i(pa + cnt);
		ia64_sync_i();
		va += max;
		sz -= max;
	}
	ia64_srlz_i();
}
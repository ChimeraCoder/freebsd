
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

#include <machine/md_var.h>
#include <machine/vmparam.h>

static u_int phys_avail_segs;

vm_paddr_t phys_avail[2 * VM_PHYSSEG_MAX + 2];

vm_paddr_t paddr_max;

long realmem;

static u_int
ia64_physmem_find(vm_paddr_t base, vm_paddr_t lim)
{
	u_int idx;

	for (idx = 0; phys_avail[idx + 1] != 0; idx += 2) {
		if (phys_avail[idx] >= lim ||
		    phys_avail[idx + 1] > base)
			break;
	}
	return (idx);
}

static int
ia64_physmem_insert(u_int idx, vm_paddr_t base, vm_paddr_t lim)
{
	u_int ridx;

	if (phys_avail_segs == VM_PHYSSEG_MAX)
		return (ENOMEM);

	ridx = phys_avail_segs * 2;
	while (idx < ridx) {
		phys_avail[ridx + 1] = phys_avail[ridx - 1];
		phys_avail[ridx] = phys_avail[ridx - 2];
		ridx -= 2;
	}
	phys_avail[idx] = base;
	phys_avail[idx + 1] = lim;
	phys_avail_segs++;
	return (0);
}

static int
ia64_physmem_remove(u_int idx)
{

	if (phys_avail_segs == 0)
		return (ENOENT);
	do {
		phys_avail[idx] = phys_avail[idx + 2];
		phys_avail[idx + 1] = phys_avail[idx + 3];
		idx += 2;
	} while (phys_avail[idx + 1] != 0);
	phys_avail_segs--;
	return (0);
}

int
ia64_physmem_add(vm_paddr_t base, vm_size_t len)
{
	vm_paddr_t lim;
	u_int idx;

	realmem += len;

	lim = base + len;
	idx = ia64_physmem_find(base, lim);
	if (phys_avail[idx] == lim) {
		phys_avail[idx] = base;
		return (0);
	}
	if (idx > 0 && phys_avail[idx - 1] == base) {
		phys_avail[idx - 1] = lim;
		return (0);
	}
	return (ia64_physmem_insert(idx, base, lim));
}

int
ia64_physmem_delete(vm_paddr_t base, vm_size_t len)
{
	vm_paddr_t lim;
	u_int idx;

	lim = base + len;
	idx = ia64_physmem_find(base, lim);
	if (phys_avail[idx] >= lim || phys_avail[idx + 1] == 0)
		return (ENOENT);
	if (phys_avail[idx] < base && phys_avail[idx + 1] > lim) {
		len = phys_avail[idx + 1] - lim;
		phys_avail[idx + 1] = base;
		base = lim;
		lim = base + len;
		return (ia64_physmem_insert(idx + 2, base, lim));
	} else {
		if (phys_avail[idx] == base)
			phys_avail[idx] = lim;
		if (phys_avail[idx + 1] == lim)
			phys_avail[idx + 1] = base;
		if (phys_avail[idx] >= phys_avail[idx + 1])
			return (ia64_physmem_remove(idx));
	}
	return (0);
}

int
ia64_physmem_fini(void)
{
	vm_paddr_t base, lim, size;
	u_int idx;

	idx = 0;
	while (phys_avail[idx + 1] != 0) {
		base = round_page(phys_avail[idx]);
		lim = trunc_page(phys_avail[idx + 1]);
		if (base < lim) {
			phys_avail[idx] = base;
			phys_avail[idx + 1] = lim;
			size = lim - base;
			physmem += atop(size);
			paddr_max = lim;
			idx += 2;
		} else
			ia64_physmem_remove(idx);
	}

	/*
	 * Round realmem to a multple of 128MB. Hopefully that compensates
	 * for any loss of DRAM that isn't accounted for in the memory map.
	 * I'm thinking legacy BIOS or VGA here. In any case, it's ok if
	 * we got it wrong, because we don't actually use realmem. It's
	 * just for show...
	 */
	size = 1U << 27;
	realmem = (realmem + size - 1) & ~(size - 1);
	realmem = atop(realmem);
	return (0);
}

int
ia64_physmem_init(void)
{

	/* Nothing to do just yet. */
	return (0);
}

int
ia64_physmem_track(vm_paddr_t base, vm_size_t len)
{

	realmem += len;
	return (0);
}

void *
ia64_physmem_alloc(vm_size_t len, vm_size_t align)
{
	vm_paddr_t base, lim, pa;
	void *ptr;
	u_int idx;

	if (phys_avail_segs == 0)
		return (NULL);

	len = round_page(len);

	/*
	 * Try and allocate with least effort.
	 */
	idx = phys_avail_segs * 2;
	while (idx > 0) {
		idx -= 2;
		base = phys_avail[idx];
		lim = phys_avail[idx + 1];

		if (lim - base < len)
			continue;

		/* First try from the end. */
		pa = lim - len;
		if ((pa & (align - 1)) == 0) {
			if (pa == base)
				ia64_physmem_remove(idx);
			else
				phys_avail[idx + 1] = pa;
			goto gotit;
		}

		/* Try from the start next. */
		pa = base;
		if ((pa & (align - 1)) == 0) {
			if (pa + len == lim)
				ia64_physmem_remove(idx);
			else
				phys_avail[idx] += len;
			goto gotit;
		}
	}

	/*
	 * Find a good segment and split it up.
	 */
	idx = phys_avail_segs * 2;
	while (idx > 0) {
		idx -= 2;
		base = phys_avail[idx];
		lim = phys_avail[idx + 1];

		pa = (base + align - 1) & ~(align - 1);
		if (pa + len <= lim) {
			ia64_physmem_delete(pa, len);
			goto gotit;
		}
	}

	/* Out of luck. */
	return (NULL);

 gotit:
	ptr = (void *)IA64_PHYS_TO_RR7(pa);
	bzero(ptr, len);
	return (ptr);
}
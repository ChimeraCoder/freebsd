
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
 * ARM machine dependent routines for kvm and minidumps.
 */

#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/fnv_hash.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <nlist.h>
#include <kvm.h>

#include <vm/vm.h>
#include <vm/vm_param.h>

#include <machine/elf.h>
#include <machine/cpufunc.h>
#include <machine/minidump.h>

#include <limits.h>

#include "kvm_private.h"

struct hpte {
	struct hpte	*next;
	uint64_t	pa;
	int64_t		off;
};

#define HPT_SIZE 1024

/* minidump must be the first field */
struct vmstate {
	int		minidump;		/* 1 = minidump mode */
	struct		minidumphdr hdr;
	void		*hpt_head[HPT_SIZE];
	uint32_t	*bitmap;
	void		*ptemap;
};

static void
hpt_insert(kvm_t *kd, uint64_t pa, int64_t off)
{
	struct hpte *hpte;
	uint32_t fnv = FNV1_32_INIT;

	fnv = fnv_32_buf(&pa, sizeof(pa), fnv);
	fnv &= (HPT_SIZE - 1);
	hpte = malloc(sizeof(*hpte));
	hpte->pa = pa;
	hpte->off = off;
	hpte->next = kd->vmst->hpt_head[fnv];
	kd->vmst->hpt_head[fnv] = hpte;
}

static int64_t
hpt_find(kvm_t *kd, uint64_t pa)
{
	struct hpte *hpte;
	uint32_t fnv = FNV1_32_INIT;

	fnv = fnv_32_buf(&pa, sizeof(pa), fnv);
	fnv &= (HPT_SIZE - 1);
	for (hpte = kd->vmst->hpt_head[fnv]; hpte != NULL; hpte = hpte->next)
		if (pa == hpte->pa)
			return (hpte->off);

	return (-1);
}

static int
inithash(kvm_t *kd, uint32_t *base, int len, off_t off)
{
	uint64_t idx, pa;
	uint32_t bit, bits;

	for (idx = 0; idx < len / sizeof(*base); idx++) {
		bits = base[idx];
		while (bits) {
			bit = ffs(bits) - 1;
			bits &= ~(1ul << bit);
			pa = (idx * sizeof(*base) * NBBY + bit) * PAGE_SIZE;
			hpt_insert(kd, pa, off);
			off += PAGE_SIZE;
		}
	}
	return (off);
}

void
_kvm_minidump_freevtop(kvm_t *kd)
{
	struct vmstate *vm = kd->vmst;

	if (vm->bitmap)
		free(vm->bitmap);
	if (vm->ptemap)
		free(vm->ptemap);
	free(vm);
	kd->vmst = NULL;
}

int
_kvm_minidump_initvtop(kvm_t *kd)
{
	struct vmstate *vmst;
	off_t off;

	vmst = _kvm_malloc(kd, sizeof(*vmst));
	if (vmst == 0) {
		_kvm_err(kd, kd->program, "cannot allocate vm");
		return (-1);
	}

	kd->vmst = vmst;
	vmst->minidump = 1;

	if (pread(kd->pmfd, &vmst->hdr,
	    sizeof(vmst->hdr), 0) != sizeof(vmst->hdr)) {
		_kvm_err(kd, kd->program, "cannot read dump header");
		return (-1);
	}

	if (strncmp(MINIDUMP_MAGIC, vmst->hdr.magic,
	    sizeof(vmst->hdr.magic)) != 0) {
		_kvm_err(kd, kd->program, "not a minidump for this platform");
		return (-1);
	}
	if (vmst->hdr.version != MINIDUMP_VERSION) {
		_kvm_err(kd, kd->program, "wrong minidump version. "
		    "Expected %d got %d", MINIDUMP_VERSION, vmst->hdr.version);
		return (-1);
	}

	/* Skip header and msgbuf */
	off = PAGE_SIZE + round_page(vmst->hdr.msgbufsize);

	vmst->bitmap = _kvm_malloc(kd, vmst->hdr.bitmapsize);
	if (vmst->bitmap == NULL) {
		_kvm_err(kd, kd->program, "cannot allocate %d bytes for "
		    "bitmap", vmst->hdr.bitmapsize);
		return (-1);
	}

	if (pread(kd->pmfd, vmst->bitmap, vmst->hdr.bitmapsize, off) !=
	    (ssize_t)vmst->hdr.bitmapsize) {
		_kvm_err(kd, kd->program, "cannot read %d bytes for page bitmap",
		    vmst->hdr.bitmapsize);
		return (-1);
	}
	off += round_page(vmst->hdr.bitmapsize);

	vmst->ptemap = _kvm_malloc(kd, vmst->hdr.ptesize);
	if (vmst->ptemap == NULL) {
		_kvm_err(kd, kd->program, "cannot allocate %d bytes for "
		    "ptemap", vmst->hdr.ptesize);
		return (-1);
	}

	if (pread(kd->pmfd, vmst->ptemap, vmst->hdr.ptesize, off) !=
	    (ssize_t)vmst->hdr.ptesize) {
		_kvm_err(kd, kd->program, "cannot read %d bytes for ptemap",
		    vmst->hdr.ptesize);
		return (-1);
	}

	off += vmst->hdr.ptesize;

	/* Build physical address hash table for sparse pages */
	inithash(kd, vmst->bitmap, vmst->hdr.bitmapsize, off);

	return (0);
}

int
_kvm_minidump_kvatop(kvm_t *kd, u_long va, off_t *pa)
{
	struct vmstate *vm;
	pt_entry_t pte;
	u_long offset, pteindex, a;
	off_t ofs;
	uint32_t *ptemap;

	if (ISALIVE(kd)) {
		_kvm_err(kd, 0, "kvm_kvatop called in live kernel!");
		return (0);
	}

	vm = kd->vmst;
	ptemap = vm->ptemap;

	if (va >= vm->hdr.kernbase) {
		pteindex = (va - vm->hdr.kernbase) >> PAGE_SHIFT;
		pte = ptemap[pteindex];
		if (!pte) {
			_kvm_err(kd, kd->program, "_kvm_vatop: pte not valid");
			goto invalid;
		}
		if ((pte & L2_TYPE_MASK) == L2_TYPE_L) {
			offset = va & L2_L_OFFSET;
			a = pte & L2_L_FRAME;
		} else if ((pte & L2_TYPE_MASK) == L2_TYPE_S) {
			offset = va & L2_S_OFFSET;
			a = pte & L2_S_FRAME;
		} else
			goto invalid;

		ofs = hpt_find(kd, a);
		if (ofs == -1) {
			_kvm_err(kd, kd->program, "_kvm_vatop: physical "
			    "address 0x%lx not in minidump", a);
			goto invalid;
		}

		*pa = ofs + offset;
		return (PAGE_SIZE - offset);

	} else
		_kvm_err(kd, kd->program, "_kvm_vatop: virtual address 0x%lx "
		    "not minidumped", va);

invalid:
	_kvm_err(kd, 0, "invalid address (0x%lx)", va);
	return (0);
}
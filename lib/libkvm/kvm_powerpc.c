
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
#include <sys/endian.h>
#include <sys/kerneldump.h>
#include <sys/mman.h>

#include <vm/vm.h>

#include <db.h>
#include <elf.h>
#include <limits.h>
#include <kvm.h>
#include <stdlib.h>
#include <string.h>

#include "kvm_private.h"

struct vmstate {
	void		*map;
	size_t		mapsz;
	size_t		dmphdrsz;
	Elf32_Ehdr	*eh;
	Elf32_Phdr	*ph;
};

static int
valid_elf_header(Elf32_Ehdr *eh)
{

	if (!IS_ELF(*eh))
		return (0);
	if (eh->e_ident[EI_CLASS] != ELFCLASS32)
		return (0);
	if (eh->e_ident[EI_DATA] != ELFDATA2MSB)
		return (0);
	if (eh->e_ident[EI_VERSION] != EV_CURRENT)
		return (0);
	if (eh->e_ident[EI_OSABI] != ELFOSABI_STANDALONE)
		return (0);
	if (be16toh(eh->e_type) != ET_CORE)
		return (0);
	if (be16toh(eh->e_machine) != EM_PPC)
		return (0);
	/* Can't think of anything else to check... */
	return (1);
}

static size_t
dump_header_size(struct kerneldumpheader *dh)
{

	if (strcmp(dh->magic, KERNELDUMPMAGIC) != 0)
		return (0);
	if (strcmp(dh->architecture, "powerpc") != 0)
		return (0);
	/* That should do it... */
	return (sizeof(*dh));
}

/*
 * Map the ELF headers into the process' address space. We do this in two
 * steps: first the ELF header itself and using that information the whole
 * set of headers.
 */
static int
powerpc_maphdrs(kvm_t *kd)
{
	struct vmstate *vm;
	size_t mapsz;

	vm = kd->vmst;

	vm->mapsz = PAGE_SIZE;
	vm->map = mmap(NULL, vm->mapsz, PROT_READ, MAP_PRIVATE, kd->pmfd, 0);
	if (vm->map == MAP_FAILED) {
		_kvm_err(kd, kd->program, "cannot map corefile");
		return (-1);
	}
	vm->dmphdrsz = 0;
	vm->eh = vm->map;
	if (!valid_elf_header(vm->eh)) {
		/*
		 * Hmmm, no ELF header. Maybe we still have a dump header.
		 * This is normal when the core file wasn't created by
		 * savecore(8), but instead was dumped over TFTP. We can
		 * easily skip the dump header...
		 */
		vm->dmphdrsz = dump_header_size(vm->map);
		if (vm->dmphdrsz == 0)
			goto inval;
		vm->eh = (void *)((uintptr_t)vm->map + vm->dmphdrsz);
		if (!valid_elf_header(vm->eh))
			goto inval;
	}
	mapsz = be16toh(vm->eh->e_phentsize) * be16toh(vm->eh->e_phnum) +
	    be32toh(vm->eh->e_phoff);
	munmap(vm->map, vm->mapsz);

	/* Map all headers. */
	vm->mapsz = vm->dmphdrsz + mapsz;
	vm->map = mmap(NULL, vm->mapsz, PROT_READ, MAP_PRIVATE, kd->pmfd, 0);
	if (vm->map == MAP_FAILED) {
		_kvm_err(kd, kd->program, "cannot map corefle headers");
		return (-1);
	}
	vm->eh = (void *)((uintptr_t)vm->map + vm->dmphdrsz);
	vm->ph = (void *)((uintptr_t)vm->eh + be32toh(vm->eh->e_phoff));
	return (0);

 inval:
	munmap(vm->map, vm->mapsz);
	vm->map = MAP_FAILED;
	_kvm_err(kd, kd->program, "invalid corefile");
	return (-1);
}

/*
 * Determine the offset within the corefile corresponding the virtual
 * address. Return the number of contiguous bytes in the corefile or
 * 0 when the virtual address is invalid.
 */
static size_t
powerpc_va2off(kvm_t *kd, u_long va, off_t *ofs)
{
	struct vmstate *vm = kd->vmst;
	Elf32_Phdr *ph;
	int nph;

	ph = vm->ph;
	nph = be16toh(vm->eh->e_phnum);
	while (nph && (va < be32toh(ph->p_vaddr) ||
	    va >= be32toh(ph->p_vaddr) + be32toh(ph->p_memsz))) {
		nph--;
		ph = (void *)((uintptr_t)ph + be16toh(vm->eh->e_phentsize));
	}
	if (nph == 0)
		return (0);

	/* Segment found. Return file offset and range. */
	*ofs = vm->dmphdrsz + be32toh(ph->p_offset) +
	    (va - be32toh(ph->p_vaddr));
	return (be32toh(ph->p_memsz) - (va - be32toh(ph->p_vaddr)));
}

void
_kvm_freevtop(kvm_t *kd)
{
	struct vmstate *vm = kd->vmst;

	if (vm == NULL)
		return;

	if (vm->eh != MAP_FAILED) {
		munmap(vm->eh, vm->mapsz);
		vm->eh = MAP_FAILED;
	}
	free(vm);
	kd->vmst = NULL;
}

int
_kvm_initvtop(kvm_t *kd)
{

	kd->vmst = (struct vmstate *)_kvm_malloc(kd, sizeof(*kd->vmst));
	if (kd->vmst == NULL) {
		_kvm_err(kd, kd->program, "out of virtual memory");
		return (-1);
	}
	if (powerpc_maphdrs(kd) == -1) {
		free(kd->vmst);
		kd->vmst = NULL;
		return (-1);
	}
	return (0);
}

int
_kvm_kvatop(kvm_t *kd, u_long va, off_t *ofs)
{
	struct vmstate *vm;

	vm = kd->vmst;
	if (vm->ph->p_paddr == ~0U)
		return ((int)powerpc_va2off(kd, va, ofs));

	_kvm_err(kd, kd->program, "Raw corefile not supported");
	return (0);
}

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

#include <sys/types.h>

#include <assert.h>
#include <dlfcn.h>
#include <stdlib.h>

#include <machine/elf.h>

#ifndef PT_IA_64_UNWIND
#define PT_IA_64_UNWIND         0x70000001
#endif

#define	SANITY	0

struct ia64_unwind_entry
{
	Elf64_Addr start;
	Elf64_Addr end;
	Elf64_Addr descr;
};

struct ia64_unwind_entry *
_Unwind_FindTableEntry(const void *pc, unsigned long *pseg, unsigned long *pgp)
{
	Dl_info info;
	Elf_Dyn *dyn;
	Elf_Ehdr *ehdr;
	Elf_Phdr *phdr;
	char *p, *p_top;
	struct ia64_unwind_entry *unw, *res;
	register unsigned long gp __asm__("gp");	/* XXX assumes gcc */
	unsigned long reloc, vaddr;
	size_t l, m, r;

	if (!dladdr(pc, &info))
		return NULL;

	ehdr = (Elf_Ehdr*)info.dli_fbase;

#if SANITY
	assert(IS_ELF(*ehdr));
	assert(ehdr->e_ident[EI_CLASS] == ELFCLASS64);
	assert(ehdr->e_ident[EI_DATA] == ELFDATA2LSB);
	assert(ehdr->e_machine == EM_IA_64);
#endif

	reloc = (ehdr->e_type == ET_DYN) ? (uintptr_t)info.dli_fbase : 0;
	*pgp = gp;
	*pseg = 0UL;
	res = NULL;

	p = (char*)info.dli_fbase + ehdr->e_phoff;
	p_top = p + ehdr->e_phnum * ehdr->e_phentsize;
	while (p < p_top) {
		phdr = (Elf_Phdr*)p;
		vaddr = phdr->p_vaddr + reloc;

		switch (phdr->p_type) {
		case PT_DYNAMIC:
			dyn = (Elf_Dyn*)vaddr;
			while (dyn->d_tag != DT_NULL) {
				if (dyn->d_tag == DT_PLTGOT) {
					*pgp = dyn->d_un.d_ptr + reloc;
					break;
				}
				dyn++;
			}
			break;
		case PT_LOAD:
			if (pc >= (void*)vaddr &&
			    pc < (void*)(vaddr + phdr->p_memsz))
				*pseg = vaddr;
			break;
		case PT_IA_64_UNWIND:
#if SANITY
			assert(*pseg != 0UL);
			assert(res == NULL);
#endif
			unw = (struct ia64_unwind_entry*)vaddr;
			l = 0;
			r = phdr->p_memsz / sizeof(struct ia64_unwind_entry);
			while (l < r) {
				m = (l + r) >> 1;
				res = unw + m;
				if (pc < (void*)(res->start + *pseg))
					r = m;
				else if (pc >= (void*)(res->end + *pseg))
					l = m + 1;
				else
					break;	/* found */
			}
			if (l >= r)
				res = NULL;
			break;
		}

		p += ehdr->e_phentsize;
	}

	return res;
}
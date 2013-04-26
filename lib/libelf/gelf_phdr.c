
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

#include <sys/limits.h>

#include <gelf.h>
#include <libelf.h>

#include "_libelf.h"

Elf32_Phdr *
elf32_getphdr(Elf *e)
{
	return (_libelf_getphdr(e, ELFCLASS32));
}

Elf64_Phdr *
elf64_getphdr(Elf *e)
{
	return (_libelf_getphdr(e, ELFCLASS64));
}

GElf_Phdr *
gelf_getphdr(Elf *e, int index, GElf_Phdr *d)
{
	int ec;
	Elf32_Ehdr *eh32;
	Elf64_Ehdr *eh64;
	Elf32_Phdr *ep32;
	Elf64_Phdr *ep64;

	if (d == NULL || e == NULL ||
	    ((ec = e->e_class) != ELFCLASS32 && ec != ELFCLASS64) ||
	    (e->e_kind != ELF_K_ELF) || index < 0) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {
		if ((eh32 = _libelf_ehdr(e, ELFCLASS32, 0)) == NULL ||
		    ((ep32 = _libelf_getphdr(e, ELFCLASS32)) == NULL))
			return (NULL);

		if (index >= eh32->e_phnum) {
			LIBELF_SET_ERROR(ARGUMENT, 0);
			return (NULL);
		}

		ep32 += index;

		d->p_type   = ep32->p_type;
		d->p_offset = ep32->p_offset;
		d->p_vaddr  = (Elf64_Addr) ep32->p_vaddr;
		d->p_paddr  = (Elf64_Addr) ep32->p_paddr;
		d->p_filesz = (Elf64_Xword) ep32->p_filesz;
		d->p_memsz  = (Elf64_Xword) ep32->p_memsz;
		d->p_flags  = ep32->p_flags;
		d->p_align  = (Elf64_Xword) ep32->p_align;

	} else {
		if ((eh64 = _libelf_ehdr(e, ELFCLASS64, 0)) == NULL ||
		    (ep64 = _libelf_getphdr(e, ELFCLASS64)) == NULL)
			return (NULL);

		if (index >= eh64->e_phnum) {
			LIBELF_SET_ERROR(ARGUMENT, 0);
			return (NULL);
		}

		ep64 += index;

		*d = *ep64;
	}

	return (d);
}

Elf32_Phdr *
elf32_newphdr(Elf *e, size_t count)
{
	return (_libelf_newphdr(e, ELFCLASS32, count));
}

Elf64_Phdr *
elf64_newphdr(Elf *e, size_t count)
{
	return (_libelf_newphdr(e, ELFCLASS64, count));
}

void *
gelf_newphdr(Elf *e, size_t count)
{
	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}
	return (_libelf_newphdr(e, e->e_class, count));
}

int
gelf_update_phdr(Elf *e, int ndx, GElf_Phdr *s)
{
	int ec, phnum;
	void *ehdr;
	Elf32_Phdr *ph32;
	Elf64_Phdr *ph64;

	if (s == NULL || e == NULL || e->e_kind != ELF_K_ELF ||
	    ((ec = e->e_class) != ELFCLASS32 && ec != ELFCLASS64)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (e->e_cmd == ELF_C_READ) {
		LIBELF_SET_ERROR(MODE, 0);
		return (0);
	}

	if ((ehdr = _libelf_ehdr(e, ec, 0)) == NULL)
		return (0);

	if (ec == ELFCLASS32)
		phnum = ((Elf32_Ehdr *) ehdr)->e_phnum;
	else
		phnum = ((Elf64_Ehdr *) ehdr)->e_phnum;

	if (ndx < 0 || ndx > phnum) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	(void) elf_flagphdr(e, ELF_C_SET, ELF_F_DIRTY);

	if (ec == ELFCLASS64) {
		ph64 = e->e_u.e_elf.e_phdr.e_phdr64 + ndx;
		*ph64 = *s;
		return (1);
	}

	ph32 = e->e_u.e_elf.e_phdr.e_phdr32 + ndx;

	ph32->p_type     =  s->p_type;
	ph32->p_flags    =  s->p_flags;
	LIBELF_COPY_U32(ph32, s, p_offset);
	LIBELF_COPY_U32(ph32, s, p_vaddr);
	LIBELF_COPY_U32(ph32, s, p_paddr);
	LIBELF_COPY_U32(ph32, s, p_filesz);
	LIBELF_COPY_U32(ph32, s, p_memsz);
	LIBELF_COPY_U32(ph32, s, p_align);

	return (1);
}
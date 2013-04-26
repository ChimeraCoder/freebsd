
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

#include <assert.h>
#include <gelf.h>
#include <libelf.h>
#include <stdlib.h>

#include "_libelf.h"

void *
_libelf_getphdr(Elf *e, int ec)
{
	size_t phnum, phentsize;
	size_t fsz, msz;
	uint64_t phoff;
	Elf32_Ehdr *eh32;
	Elf64_Ehdr *eh64;
	void *ehdr, *phdr;
	int (*xlator)(char *_d, size_t _dsz, char *_s, size_t _c, int _swap);

	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if ((phdr = (ec == ELFCLASS32 ?
		 (void *) e->e_u.e_elf.e_phdr.e_phdr32 :
		 (void *) e->e_u.e_elf.e_phdr.e_phdr64)) != NULL)
		return (phdr);

	/*
	 * Check the PHDR related fields in the EHDR for sanity.
	 */

	if ((ehdr = _libelf_ehdr(e, ec, 0)) == NULL)
		return (NULL);

	phnum = e->e_u.e_elf.e_nphdr;

	if (ec == ELFCLASS32) {
		eh32      = (Elf32_Ehdr *) ehdr;
		phentsize = eh32->e_phentsize;
		phoff     = (uint64_t) eh32->e_phoff;
	} else {
		eh64      = (Elf64_Ehdr *) ehdr;
		phentsize = eh64->e_phentsize;
		phoff     = (uint64_t) eh64->e_phoff;
	}

	fsz = gelf_fsize(e, ELF_T_PHDR, phnum, e->e_version);

	assert(fsz > 0);

	if ((uint64_t) e->e_rawsize < (phoff + fsz)) {
		LIBELF_SET_ERROR(HEADER, 0);
		return (NULL);
	}

	msz = _libelf_msize(ELF_T_PHDR, ec, EV_CURRENT);

	assert(msz > 0);

	if ((phdr = calloc(phnum, msz)) == NULL) {
		LIBELF_SET_ERROR(RESOURCE, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32)
		e->e_u.e_elf.e_phdr.e_phdr32 = phdr;
	else
		e->e_u.e_elf.e_phdr.e_phdr64 = phdr;


	xlator = _libelf_get_translator(ELF_T_PHDR, ELF_TOMEMORY, ec);
	(*xlator)(phdr, phnum * msz, e->e_rawfile + phoff, phnum,
	    e->e_byteorder != LIBELF_PRIVATE(byteorder));

	return (phdr);
}

void *
_libelf_newphdr(Elf *e, int ec, size_t count)
{
	void *ehdr, *newphdr, *oldphdr;
	size_t msz;

	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if ((ehdr = _libelf_ehdr(e, ec, 0)) == NULL) {
		LIBELF_SET_ERROR(SEQUENCE, 0);
		return (NULL);
	}

	assert(e->e_class == ec);
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);
	assert(e->e_version == EV_CURRENT);

	msz = _libelf_msize(ELF_T_PHDR, ec, e->e_version);

	assert(msz > 0);

	newphdr = NULL;
	if (count > 0 && (newphdr = calloc(count, msz)) == NULL) {
		LIBELF_SET_ERROR(RESOURCE, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {
		if ((oldphdr = (void *) e->e_u.e_elf.e_phdr.e_phdr32) != NULL)
			free(oldphdr);
		e->e_u.e_elf.e_phdr.e_phdr32 = (Elf32_Phdr *) newphdr;
	} else {
		if ((oldphdr = (void *) e->e_u.e_elf.e_phdr.e_phdr64) != NULL)
			free(oldphdr);
		e->e_u.e_elf.e_phdr.e_phdr64 = (Elf64_Phdr *) newphdr;
	}

	e->e_u.e_elf.e_nphdr = count;

	elf_flagphdr(e, ELF_C_SET, ELF_F_DIRTY);

	return (newphdr);
}
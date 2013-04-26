
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

#include <assert.h>
#include <gelf.h>
#include <libelf.h>
#include <string.h>

#include "_libelf.h"

Elf32_Ehdr *
elf32_getehdr(Elf *e)
{
	return (_libelf_ehdr(e, ELFCLASS32, 0));
}

Elf64_Ehdr *
elf64_getehdr(Elf *e)
{
	return (_libelf_ehdr(e, ELFCLASS64, 0));
}

GElf_Ehdr *
gelf_getehdr(Elf *e, GElf_Ehdr *d)
{
	int ec;
	Elf32_Ehdr *eh32;
	Elf64_Ehdr *eh64;

	if (d == NULL || e == NULL ||
	    ((ec = e->e_class) != ELFCLASS32 && ec != ELFCLASS64)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {
		if ((eh32 = _libelf_ehdr(e, ELFCLASS32, 0)) == NULL)
			return (NULL);

		(void) memcpy(d->e_ident, eh32->e_ident, sizeof(eh32->e_ident));
		d->e_type		= eh32->e_type;
		d->e_machine		= eh32->e_machine;
		d->e_version		= eh32->e_version;
		d->e_entry		= eh32->e_entry;
		d->e_phoff		= eh32->e_phoff;
		d->e_shoff		= eh32->e_shoff;
		d->e_flags		= eh32->e_flags;
		d->e_ehsize		= eh32->e_ehsize;
		d->e_phentsize		= eh32->e_phentsize;
		d->e_phnum		= eh32->e_phnum;
		d->e_shentsize		= eh32->e_shentsize;
		d->e_shnum		= eh32->e_shnum;
		d->e_shstrndx		= eh32->e_shstrndx;

		return (d);
	}

	assert(ec == ELFCLASS64);

	if ((eh64 = _libelf_ehdr(e, ELFCLASS64, 0)) == NULL)
		return (NULL);
	*d = *eh64;

	return (d);
}

Elf32_Ehdr *
elf32_newehdr(Elf *e)
{
	return (_libelf_ehdr(e, ELFCLASS32, 1));
}

Elf64_Ehdr *
elf64_newehdr(Elf *e)
{
	return (_libelf_ehdr(e, ELFCLASS64, 1));
}

void *
gelf_newehdr(Elf *e, int ec)
{
	if (e != NULL &&
	    (ec == ELFCLASS32 || ec == ELFCLASS64))
		return (_libelf_ehdr(e, ec, 1));

	LIBELF_SET_ERROR(ARGUMENT, 0);
	return (NULL);
}

int
gelf_update_ehdr(Elf *e, GElf_Ehdr *s)
{
	int ec;
	void *ehdr;
	Elf32_Ehdr *eh32;
	Elf64_Ehdr *eh64;

	if (s== NULL || e == NULL || e->e_kind != ELF_K_ELF ||
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

	(void) elf_flagehdr(e, ELF_C_SET, ELF_F_DIRTY);

	if (ec == ELFCLASS64) {
		eh64 = (Elf64_Ehdr *) ehdr;
		*eh64 = *s;
		return (1);
	}

	eh32 = (Elf32_Ehdr *) ehdr;

	(void) memcpy(eh32->e_ident, s->e_ident, sizeof(eh32->e_ident));

	eh32->e_type      = s->e_type;
	eh32->e_machine   = s->e_machine;
	eh32->e_version   = s->e_version;
	LIBELF_COPY_U32(eh32, s, e_entry);
	LIBELF_COPY_U32(eh32, s, e_phoff);
	LIBELF_COPY_U32(eh32, s, e_shoff);
	eh32->e_flags     = s->e_flags;
	eh32->e_ehsize    = s->e_ehsize;
	eh32->e_phentsize = s->e_phentsize;
	eh32->e_phnum     = s->e_phnum;
	eh32->e_shentsize = s->e_shentsize;
	eh32->e_shnum     = s->e_shnum;
	eh32->e_shstrndx  = s->e_shstrndx;

	return (1);
}
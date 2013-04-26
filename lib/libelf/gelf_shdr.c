
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

#include "_libelf.h"

Elf32_Shdr *
elf32_getshdr(Elf_Scn *s)
{
	return (_libelf_getshdr(s, ELFCLASS32));
}

Elf64_Shdr *
elf64_getshdr(Elf_Scn *s)
{
	return (_libelf_getshdr(s, ELFCLASS64));
}

GElf_Shdr *
gelf_getshdr(Elf_Scn *s, GElf_Shdr *d)
{
	int ec;
	void *sh;
	Elf32_Shdr *sh32;
	Elf64_Shdr *sh64;

	if (d == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if ((sh = _libelf_getshdr(s, ELFCLASSNONE)) == NULL)
		return (NULL);

	ec = s->s_elf->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32) {
		sh32 = (Elf32_Shdr *) sh;

		d->sh_name      = sh32->sh_name;
		d->sh_type      = sh32->sh_type;
		d->sh_flags     = (Elf64_Xword) sh32->sh_flags;
		d->sh_addr      = (Elf64_Addr) sh32->sh_addr;
		d->sh_offset    = (Elf64_Off) sh32->sh_offset;
		d->sh_size      = (Elf64_Xword) sh32->sh_size;
		d->sh_link      = sh32->sh_link;
		d->sh_info      = sh32->sh_info;
		d->sh_addralign = (Elf64_Xword) sh32->sh_addralign;
		d->sh_entsize   = (Elf64_Xword) sh32->sh_entsize;
	} else {
		sh64 = (Elf64_Shdr *) sh;
		*d = *sh64;
	}

	return (d);
}

int
gelf_update_shdr(Elf_Scn *scn, GElf_Shdr *s)
{
	int ec;
	Elf *e;
	Elf32_Shdr *sh32;


	if (s == NULL || scn == NULL || (e = scn->s_elf) == NULL ||
	    e->e_kind != ELF_K_ELF ||
	    ((ec = e->e_class) != ELFCLASS32 && ec != ELFCLASS64)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (e->e_cmd == ELF_C_READ) {
		LIBELF_SET_ERROR(MODE, 0);
		return (0);
	}

	(void) elf_flagscn(scn, ELF_C_SET, ELF_F_DIRTY);

	if (ec == ELFCLASS64) {
		scn->s_shdr.s_shdr64 = *s;
		return (1);
	}

	sh32 = &scn->s_shdr.s_shdr32;

	sh32->sh_name	 =  s->sh_name;
	sh32->sh_type	 =  s->sh_type;
	LIBELF_COPY_U32(sh32, s, sh_flags);
	LIBELF_COPY_U32(sh32, s, sh_addr);
	LIBELF_COPY_U32(sh32, s, sh_offset);
	LIBELF_COPY_U32(sh32, s, sh_size);
	sh32->sh_link	 =  s->sh_link;
	sh32->sh_info	 =  s->sh_info;
	LIBELF_COPY_U32(sh32, s, sh_addralign);
	LIBELF_COPY_U32(sh32, s, sh_entsize);

	return (1);
}
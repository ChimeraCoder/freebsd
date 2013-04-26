
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

#include "_libelf.h"

GElf_Sym *
gelf_getsym(Elf_Data *d, int ndx, GElf_Sym *dst)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Sym *sym32;
	Elf64_Sym *sym64;
	size_t msz;
	uint32_t sh_type;

	if (d == NULL || ndx < 0 || dst == NULL ||
	    (scn = d->d_scn) == NULL ||
	    (e = scn->s_elf) == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	ec = e->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32)
		sh_type = scn->s_shdr.s_shdr32.sh_type;
	else
		sh_type = scn->s_shdr.s_shdr64.sh_type;

	if (_libelf_xlate_shtype(sh_type) != ELF_T_SYM) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	msz = _libelf_msize(ELF_T_SYM, ec, e->e_version);

	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {

		sym32 = (Elf32_Sym *) d->d_buf + ndx;

		dst->st_name  = sym32->st_name;
		dst->st_value = (Elf64_Addr) sym32->st_value;
		dst->st_size  = (Elf64_Xword) sym32->st_size;
		dst->st_info  = ELF64_ST_INFO(ELF32_ST_BIND(sym32->st_info),
		    ELF32_ST_TYPE(sym32->st_info));
		dst->st_other = sym32->st_other;
		dst->st_shndx = sym32->st_shndx;
	} else {

		sym64 = (Elf64_Sym *) d->d_buf + ndx;

		*dst = *sym64;
	}

	return (dst);
}

int
gelf_update_sym(Elf_Data *d, int ndx, GElf_Sym *gs)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Sym *sym32;
	Elf64_Sym *sym64;
	size_t msz;
	uint32_t sh_type;

	if (d == NULL || ndx < 0 || gs == NULL ||
	    (scn = d->d_scn) == NULL ||
	    (e = scn->s_elf) == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	ec = e->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32)
		sh_type = scn->s_shdr.s_shdr32.sh_type;
	else
		sh_type = scn->s_shdr.s_shdr64.sh_type;

	if (_libelf_xlate_shtype(sh_type) != ELF_T_SYM) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	msz = _libelf_msize(ELF_T_SYM, ec, e->e_version);
	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (ec == ELFCLASS32) {
		sym32 = (Elf32_Sym *) d->d_buf + ndx;

		sym32->st_name  = gs->st_name;
		sym32->st_info  = gs->st_info;
		sym32->st_other = gs->st_other;
		sym32->st_shndx = gs->st_shndx;

		LIBELF_COPY_U32(sym32, gs, st_value);
		LIBELF_COPY_U32(sym32, gs, st_size);
	} else {
		sym64 = (Elf64_Sym *) d->d_buf + ndx;

		*sym64 = *gs;
	}

	return (1);
}
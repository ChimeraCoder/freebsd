
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

GElf_Rela *
gelf_getrela(Elf_Data *d, int ndx, GElf_Rela *dst)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Rela *rela32;
	Elf64_Rela *rela64;
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

	if (_libelf_xlate_shtype(sh_type) != ELF_T_RELA) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	msz = _libelf_msize(ELF_T_RELA, ec, e->e_version);

	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {
		rela32 = (Elf32_Rela *) d->d_buf + ndx;

		dst->r_offset = (Elf64_Addr) rela32->r_offset;
		dst->r_info   = ELF64_R_INFO(
		    (Elf64_Xword) ELF32_R_SYM(rela32->r_info),
		    ELF32_R_TYPE(rela32->r_info));
		dst->r_addend = (Elf64_Sxword) rela32->r_addend;

	} else {

		rela64 = (Elf64_Rela *) d->d_buf + ndx;

		*dst = *rela64;
	}

	return (dst);
}

int
gelf_update_rela(Elf_Data *d, int ndx, GElf_Rela *dr)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Rela *rela32;
	Elf64_Rela *rela64;
	size_t msz;
	uint32_t sh_type;

	if (d == NULL || ndx < 0 || dr == NULL ||
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

	if (_libelf_xlate_shtype(sh_type) != ELF_T_RELA) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	msz = _libelf_msize(ELF_T_RELA, ec, e->e_version);
	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (ec == ELFCLASS32) {
		rela32 = (Elf32_Rela *) d->d_buf + ndx;

		LIBELF_COPY_U32(rela32, dr, r_offset);

		if (ELF64_R_SYM(dr->r_info) > ELF32_R_SYM(~0UL) ||
		    ELF64_R_TYPE(dr->r_info) > ELF32_R_TYPE(~0U)) {
			LIBELF_SET_ERROR(RANGE, 0);
			return (0);
		}
		rela32->r_info = ELF32_R_INFO(ELF64_R_SYM(dr->r_info),
		    ELF64_R_TYPE(dr->r_info));

		LIBELF_COPY_S32(rela32, dr, r_addend);
	} else {
		rela64 = (Elf64_Rela *) d->d_buf + ndx;

		*rela64 = *dr;
	}

	return (1);
}
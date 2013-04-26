
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

GElf_Dyn *
gelf_getdyn(Elf_Data *d, int ndx, GElf_Dyn *dst)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Dyn *dyn32;
	Elf64_Dyn *dyn64;
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

	if (_libelf_xlate_shtype(sh_type) != ELF_T_DYN) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	msz = _libelf_msize(ELF_T_DYN, ec, e->e_version);

	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {
		dyn32 = (Elf32_Dyn *) d->d_buf + ndx;

		dst->d_tag      = dyn32->d_tag;
		dst->d_un.d_val = (Elf64_Xword) dyn32->d_un.d_val;

	} else {

		dyn64 = (Elf64_Dyn *) d->d_buf + ndx;

		*dst = *dyn64;
	}

	return (dst);
}

int
gelf_update_dyn(Elf_Data *d, int ndx, GElf_Dyn *ds)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Dyn *dyn32;
	Elf64_Dyn *dyn64;
	size_t msz;
	uint32_t sh_type;

	if (d == NULL || ndx < 0 || ds == NULL ||
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

	if (_libelf_xlate_shtype(sh_type) != ELF_T_DYN) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	msz = _libelf_msize(ELF_T_DYN, ec, e->e_version);
	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (ec == ELFCLASS32) {
		dyn32 = (Elf32_Dyn *) d->d_buf + ndx;

		LIBELF_COPY_S32(dyn32, ds, d_tag);
		LIBELF_COPY_U32(dyn32, ds, d_un.d_val);
	} else {
		dyn64 = (Elf64_Dyn *) d->d_buf + ndx;

		*dyn64 = *ds;
	}

	return (1);
}
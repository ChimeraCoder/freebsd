
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
gelf_getsymshndx(Elf_Data *d, Elf_Data *id, int ndx, GElf_Sym *dst,
    Elf32_Word *shindex)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	size_t msz;
	uint32_t sh_type;

	if (gelf_getsym(d, ndx, dst) == 0)
		return (NULL);

	if (id == NULL || (scn = id->d_scn) == NULL ||
	    (e = scn->s_elf) == NULL || (e != d->d_scn->s_elf) ||
	    shindex == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	ec = e->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32)
		sh_type = scn->s_shdr.s_shdr32.sh_type;
	else
		sh_type = scn->s_shdr.s_shdr64.sh_type;

	if (_libelf_xlate_shtype(sh_type) != ELF_T_WORD ||
	   id->d_type != ELF_T_WORD) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	msz = _libelf_msize(ELF_T_WORD, ec, e->e_version);

	assert(msz > 0);

	if (msz * ndx >= id->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	*shindex = ((Elf32_Word *) id->d_buf)[ndx];

	return (dst);
}

int
gelf_update_symshndx(Elf_Data *d, Elf_Data *id, int ndx, GElf_Sym *gs,
    Elf32_Word xindex)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	size_t msz;
	uint32_t sh_type;

	if (gelf_update_sym(d, ndx, gs) == 0)
		return (0);

	if (id == NULL || (scn = id->d_scn) == NULL ||
	    (e = scn->s_elf) == NULL || (e != d->d_scn->s_elf)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	ec = e->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32)
		sh_type = scn->s_shdr.s_shdr32.sh_type;
	else
		sh_type = scn->s_shdr.s_shdr64.sh_type;

	if (_libelf_xlate_shtype(sh_type) != ELF_T_WORD ||
	    d->d_type != ELF_T_WORD) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	msz = _libelf_msize(ELF_T_WORD, ec, e->e_version);
	assert(msz > 0);

	if (msz * ndx >= id->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	*(((Elf32_Word *) id->d_buf) + ndx) = xindex;

	return (1);
}
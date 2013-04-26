
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
#include <libelf.h>

#include "_libelf.h"

/*
 * Retrieve section #0, allocating a new section if needed.
 */
static Elf_Scn *
_libelf_getscn0(Elf *e)
{
	Elf_Scn *s;

	if ((s = STAILQ_FIRST(&e->e_u.e_elf.e_scn)) != NULL)
		return (s);

	return (_libelf_allocate_scn(e, (size_t) SHN_UNDEF));
}

int
_libelf_setshnum(Elf *e, void *eh, int ec, size_t shnum)
{
	Elf_Scn *scn;

	if (shnum >= SHN_LORESERVE) {
		if ((scn = _libelf_getscn0(e)) == NULL)
			return (0);

		assert(scn->s_ndx == SHN_UNDEF);

		if (ec == ELFCLASS32)
			scn->s_shdr.s_shdr32.sh_size = shnum;
		else
			scn->s_shdr.s_shdr64.sh_size = shnum;

		(void) elf_flagshdr(scn, ELF_C_SET, ELF_F_DIRTY);

		shnum = 0;
	}

	if (ec == ELFCLASS32)
		((Elf32_Ehdr *) eh)->e_shnum = shnum;
	else
		((Elf64_Ehdr *) eh)->e_shnum = shnum;


	return (1);
}

int
_libelf_setshstrndx(Elf *e, void *eh, int ec, size_t shstrndx)
{
	Elf_Scn *scn;

	if (shstrndx >= SHN_LORESERVE) {
		if ((scn = _libelf_getscn0(e)) == NULL)
			return (0);

		assert(scn->s_ndx == SHN_UNDEF);

		if (ec == ELFCLASS32)
			scn->s_shdr.s_shdr32.sh_link = shstrndx;
		else
			scn->s_shdr.s_shdr64.sh_link = shstrndx;

		(void) elf_flagshdr(scn, ELF_C_SET, ELF_F_DIRTY);

		shstrndx = SHN_XINDEX;
	}

	if (ec == ELFCLASS32)
		((Elf32_Ehdr *) eh)->e_shstrndx = shstrndx;
	else
		((Elf64_Ehdr *) eh)->e_shstrndx = shstrndx;

	return (1);
}

int
_libelf_setphnum(Elf *e, void *eh, int ec, size_t phnum)
{
	Elf_Scn *scn;

	if (phnum >= PN_XNUM) {
		if ((scn = _libelf_getscn0(e)) == NULL)
			return (0);

		assert(scn->s_ndx == SHN_UNDEF);

		if (ec == ELFCLASS32)
			scn->s_shdr.s_shdr32.sh_info = phnum;
		else
			scn->s_shdr.s_shdr64.sh_info = phnum;

		(void) elf_flagshdr(scn, ELF_C_SET, ELF_F_DIRTY);

		phnum = PN_XNUM;
	}

	if (ec == ELFCLASS32)
		((Elf32_Ehdr *) eh)->e_phnum = phnum;
	else
		((Elf64_Ehdr *) eh)->e_phnum = phnum;

	return (1);
}
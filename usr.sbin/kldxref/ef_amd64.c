
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
#include <machine/elf.h>

#include <err.h>
#include <errno.h>
#include <string.h>

#include "ef.h"

/*
 * Apply relocations to the values we got from the file. `relbase' is the
 * target relocation address of the section, and `dataoff' is the target
 * relocation address of the data in `dest'.
 */
int
ef_reloc(struct elf_file *ef, const void *reldata, int reltype, Elf_Off relbase,
    Elf_Off dataoff, size_t len, void *dest)
{
	Elf64_Addr *where, val;
	Elf32_Addr *where32, val32;
	Elf_Addr addend, addr;
	Elf_Size rtype, symidx;
	const Elf_Rel *rel;
	const Elf_Rela *rela;

	switch (reltype) {
	case EF_RELOC_REL:
		rel = (const Elf_Rel *)reldata;
		where = (Elf_Addr *)(dest + relbase + rel->r_offset - dataoff);
		addend = 0;
		rtype = ELF_R_TYPE(rel->r_info);
		symidx = ELF_R_SYM(rel->r_info);
		break;
	case EF_RELOC_RELA:
		rela = (const Elf_Rela *)reldata;
		where = (Elf_Addr *)(dest + relbase + rela->r_offset - dataoff);
		addend = rela->r_addend;
		rtype = ELF_R_TYPE(rela->r_info);
		symidx = ELF_R_SYM(rela->r_info);
		break;
	default:
		return (EINVAL);
	}

	if ((char *)where < (char *)dest || (char *)where >= (char *)dest + len)
		return (0);

	if (reltype == EF_RELOC_REL) {
		/* Addend is 32 bit on 32 bit relocs */
		switch (rtype) {
		case R_X86_64_PC32:
		case R_X86_64_32S:
			addend = *(Elf32_Addr *)where;
			break;
		default:
			addend = *where;
			break;
		}
	}

	switch (rtype) {
	case R_X86_64_NONE:	/* none */
		break;
	case R_X86_64_64:	/* S + A */
		addr = EF_SYMADDR(ef, symidx);
		val = addr + addend;
		*where = val;
		break;
	case R_X86_64_32S:	/* S + A sign extend */
		addr = EF_SYMADDR(ef, symidx);
		val32 = (Elf32_Addr)(addr + addend);
		where32 = (Elf32_Addr *)where;
		*where32 = val32;
		break;
	case R_X86_64_GLOB_DAT:	/* S */
		addr = EF_SYMADDR(ef, symidx);
		*where = addr;
		break;
	case R_X86_64_RELATIVE:	/* B + A */
		addr = (Elf_Addr)addend + relbase;
		val = addr;
		*where = val;
		break;
	default:
		warnx("unhandled relocation type %d", (int)rtype);
	}
	return (0);
}
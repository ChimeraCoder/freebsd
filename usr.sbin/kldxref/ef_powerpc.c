
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

#include <stdio.h>

/*
 * Apply relocations to the values obtained from the file. `relbase' is the
 * target relocation address of the section, and `dataoff/len' is the region
 * that is to be relocated, and has been copied to *dest
 */
int
ef_reloc(struct elf_file *ef, const void *reldata, int reltype, Elf_Off relbase,
    Elf_Off dataoff, size_t len, void *dest)
{
        Elf_Addr *where, addend;
        Elf_Size rtype, symidx;
        const Elf_Rela *rela;

	if (reltype != EF_RELOC_RELA)
		return (EINVAL);

	rela = (const Elf_Rela *)reldata;
	where = (Elf_Addr *) ((Elf_Off)dest - dataoff + rela->r_offset);
	addend = rela->r_addend;
	rtype = ELF_R_TYPE(rela->r_info);
	symidx = ELF_R_SYM(rela->r_info);

	 if ((char *)where < (char *)dest || (char *)where >= (char *)dest + len)
                return (0);

	switch(rtype) {
	case R_PPC_RELATIVE: /* word32 B + A */
		*where = relbase + addend;
		break;
	default:
		warnx("unhandled relocation type %d", rtype);
	}
	return (0);
}
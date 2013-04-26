
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
	const Elf_Rela *a;
	Elf_Size w;

	switch (reltype) {
	case EF_RELOC_RELA:
		a = reldata;
		if (relbase + a->r_offset >= dataoff && relbase + a->r_offset <
		    dataoff + len) {
			switch (ELF_R_TYPE(a->r_info)) {
			case R_SPARC_RELATIVE:
				w = a->r_addend + relbase;
				memcpy((u_char *)dest + (relbase + a->r_offset -
				    dataoff), &w, sizeof(w));
				break;
			default:
				warnx("unhandled relocation type %u",
				    (unsigned int)ELF_R_TYPE(a->r_info));
				break;
			}
		}
		break;
	}
	return (0);
}
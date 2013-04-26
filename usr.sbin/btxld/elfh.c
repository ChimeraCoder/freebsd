
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
#include <sys/endian.h>

#include <stddef.h>
#include "elfh.h"

#define SET_ME	0xeeeeeeee    /* filled in by btxld */

/*
 * ELF header template.
 */
const struct elfh elfhdr = {
    {
	{
	    ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3,     /* e_ident */
	    ELFCLASS32, ELFDATA2LSB, EV_CURRENT, 0,
	    'F', 'r', 'e', 'e', 'B', 'S', 'D', 0
	},
	htole16(ET_EXEC),			    /* e_type */
	htole16(EM_386),			    /* e_machine */
	htole32(EV_CURRENT),			    /* e_version */
	htole32(SET_ME),			    /* e_entry */
	htole32(offsetof(struct elfh, p)),	    /* e_phoff */
	htole32(offsetof(struct elfh, sh)),	    /* e_shoff */
	0,					    /* e_flags */
	htole16(sizeof(elfhdr.e)),		    /* e_ehsize */
	htole16(sizeof(elfhdr.p[0])),		    /* e_phentsize */
	htole16(sizeof(elfhdr.p) / sizeof(elfhdr.p[0])), /* e_phnum */
	htole16(sizeof(elfhdr.sh[0])),		    /* e_shentsize */
	htole16(sizeof(elfhdr.sh) / sizeof(elfhdr.sh[0])), /* e_shnum */
	htole16(1)				    /* e_shstrndx */
    },
    {
	{
	    htole32(PT_LOAD),			    /* p_type */
	    htole32(sizeof(elfhdr)),		    /* p_offset */
	    htole32(SET_ME),			    /* p_vaddr */
	    htole32(SET_ME),			    /* p_paddr */
	    htole32(SET_ME),			    /* p_filesz */
	    htole32(SET_ME),			    /* p_memsz */
	    htole32(PF_R | PF_X),		    /* p_flags */
	    htole32(0x1000)			    /* p_align */
	},
	{
	    htole32(PT_LOAD),			    /* p_type */
	    htole32(SET_ME),			    /* p_offset */
	    htole32(SET_ME),			    /* p_vaddr */
	    htole32(SET_ME),			    /* p_paddr */
	    htole32(SET_ME),			    /* p_filesz */
	    htole32(SET_ME),			    /* p_memsz */
	    htole32(PF_R | PF_W),		    /* p_flags */
	    htole32(0x1000)			    /* p_align */
	}
    },
    {
	{
	    0, htole32(SHT_NULL), 0, 0, 0, 0, htole32(SHN_UNDEF), 0, 0, 0
	},
	{
	    htole32(1),				    /* sh_name */
	    htole32(SHT_STRTAB), 		    /* sh_type */
	    0,					    /* sh_flags */
	    0,					    /* sh_addr */
	    htole32(offsetof(struct elfh, shstrtab)), /* sh_offset */
	    htole32(sizeof(elfhdr.shstrtab)),	    /* sh_size */
	    htole32(SHN_UNDEF),			    /* sh_link */
	    0,					    /* sh_info */
	    htole32(1),				    /* sh_addralign */
	    0					    /* sh_entsize */
	},
	{
	    htole32(0xb),			    /* sh_name */
	    htole32(SHT_PROGBITS),		    /* sh_type */
	    htole32(SHF_EXECINSTR | SHF_ALLOC),	    /* sh_flags */
	    htole32(SET_ME),			    /* sh_addr */
	    htole32(SET_ME),			    /* sh_offset */
	    htole32(SET_ME),			    /* sh_size */
	    htole32(SHN_UNDEF),			    /* sh_link */
	    0,					    /* sh_info */
	    htole32(4),				    /* sh_addralign */
	    0					    /* sh_entsize */
	},
	{
	    htole32(0x11),			    /* sh_name */
	    htole32(SHT_PROGBITS),		    /* sh_type */
	    htole32(SHF_ALLOC | SHF_WRITE),	    /* sh_flags */
	    htole32(SET_ME),			    /* sh_addr */
	    htole32(SET_ME),			    /* sh_offset */
	    htole32(SET_ME),			    /* sh_size */
	    htole32(SHN_UNDEF),			    /* sh_link */
	    0,					    /* sh_info */
	    htole32(4),				    /* sh_addralign */
	    0					    /* sh_entsize */
	}
    },
    "\0.shstrtab\0.text\0.data" 		    /* shstrtab */
};
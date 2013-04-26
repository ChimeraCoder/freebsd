
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

#include <sys/endian.h>
#include <stddef.h>
#include "elfhdr.h"

#define KGZ_FIX_NSIZE	0	/* Run-time fixup */

/*
 * Relocatable header template.
 */
const struct kgz_elfhdr elfhdr = {
    /* ELF header */
    {
	{
	    ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, 	/* e_ident */
	    ELFCLASS32, ELFDATA2LSB, EV_CURRENT, 0,
	    'F', 'r', 'e', 'e', 'B', 'S', 'D', 0
	},
	htole16(ET_EXEC),				/* e_type */
	htole16(EM_386),				/* e_machine */
	htole32(EV_CURRENT),				/* e_version */
	0,						/* e_entry */
	0,						/* e_phoff */
	htole32(offsetof(struct kgz_elfhdr, sh)),	/* e_shoff */
	0,						/* e_flags */
	htole16(sizeof(Elf32_Ehdr)),			/* e_ehsize */
	0,						/* e_phentsize */
	0,						/* e_phnum */
	htole16(sizeof(Elf32_Shdr)),			/* e_shentsize */
	htole16(KGZ_SHNUM),				/* e_shnum */
	htole16(KGZ_SH_SHSTRTAB)			/* e_shstrndx */
    },
    /* Section header */
    {
	{
	    0,						/* sh_name */
	    htole32(SHT_NULL),				/* sh_type */
	    0,						/* sh_flags */
	    0,						/* sh_addr */
	    0,						/* sh_offset */
	    0,						/* sh_size */
	    htole32(SHN_UNDEF),				/* sh_link */
	    0,						/* sh_info */
	    0,						/* sh_addralign */
	    0						/* sh_entsize */
	},
	{
	    htole32(offsetof(struct kgz_shstrtab, symtab)), /* sh_name */
	    htole32(SHT_SYMTAB),			/* sh_type */
	    0,						/* sh_flags */
	    0,						/* sh_addr */
	    htole32(offsetof(struct kgz_elfhdr, st)),	/* sh_offset */
	    htole32(sizeof(Elf32_Sym) * KGZ_STNUM),	/* sh_size */
	    htole32(KGZ_SH_STRTAB),			/* sh_link */
	    htole32(1),					/* sh_info */
	    htole32(4),					/* sh_addralign */
	    htole32(sizeof(Elf32_Sym))			/* sh_entsize */
	},
	{
	    htole32(offsetof(struct kgz_shstrtab, shstrtab)), /* sh_name */
	    htole32(SHT_STRTAB),			/* sh_type */
	    0,						/* sh_flags */
	    0,						/* sh_addr */
	    htole32(offsetof(struct kgz_elfhdr, shstrtab)), /* sh_offset */
	    htole32(sizeof(struct kgz_shstrtab)),	/* sh_size */
	    htole32(SHN_UNDEF),				/* sh_link */
	    0,						/* sh_info */
	    htole32(1),					/* sh_addralign */
	    0						/* sh_entsize */
	},
	{
	    htole32(offsetof(struct kgz_shstrtab, strtab)), /* sh_name */
	    htole32(SHT_STRTAB),			/* sh_type */
	    0,						/* sh_flags */
	    0,						/* sh_addr */
	    htole32(offsetof(struct kgz_elfhdr, strtab)), /* sh_offset */
	    htole32(sizeof(struct kgz_strtab)),		/* sh_size */
	    htole32(SHN_UNDEF),				/* sh_link */
	    0,						/* sh_info */
	    htole32(1),					/* sh_addralign */
	    0						/* sh_entsize */
	},
	{
	    htole32(offsetof(struct kgz_shstrtab, data)), /* sh_name */
	    htole32(SHT_PROGBITS),			/* sh_type */
	    htole32(SHF_ALLOC | SHF_WRITE),		/* sh_flags */
	    0,						/* sh_addr */
	    htole32(sizeof(struct kgz_elfhdr)),		/* sh_offset */
	    htole32(sizeof(struct kgz_hdr) + KGZ_FIX_NSIZE), /* sh_size */
	    htole32(SHN_UNDEF),				/* sh_link */
	    0,						/* sh_info */
	    htole32(4),					/* sh_addralign */
	    0						/* sh_entsize */
	}
    },
    /* Symbol table */
    {
	{
	    0,						/* st_name */
	    0,						/* st_value */
	    0,						/* st_size */
	    0,						/* st_info */
	    0,						/* st_other */
	    htole16(SHN_UNDEF)				/* st_shndx */
	},
	{
	    htole32(offsetof(struct kgz_strtab, kgz)),	/* st_name */
	    0,						/* st_value */
	    htole32(sizeof(struct kgz_hdr)),		/* st_size */
	    ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT),	/* st_info */
	    0,						/* st_other */
	    htole16(KGZ_SH_DATA)			/* st_shndx */
	},
	{
	    htole32(offsetof(struct kgz_strtab, kgz_ndata)), /* st_name */
	    htole32(sizeof(struct kgz_hdr)),		/* st_value */
	    htole32(KGZ_FIX_NSIZE),			/* st_size */
	    ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT),	/* st_info */
	    0,						/* st_other */
	    htole16(KGZ_SH_DATA)			/* st_shndx */
	}
    },
    /* Section header string table */
    {
	KGZ_SHSTR_ZERO, 				/* zero */
	KGZ_SHSTR_SYMTAB,				/* symtab */
	KGZ_SHSTR_SHSTRTAB,				/* shstrtab */
	KGZ_SHSTR_STRTAB,				/* strtab */
	KGZ_SHSTR_DATA					/* data */
    },
    /* String table */
    {
	KGZ_STR_ZERO,					/* zero */
	KGZ_STR_KGZ,					/* kgz */
	KGZ_STR_KGZ_NDATA				/* kgz_ndata */
    }
};
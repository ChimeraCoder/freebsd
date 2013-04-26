
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

#include <libelf.h>
#include <osreldate.h>

#include "_libelf.h"

int
_libelf_xlate_shtype(uint32_t sht)
{
	switch (sht) {
	case SHT_DYNAMIC:
		return (ELF_T_DYN);
	case SHT_DYNSYM:
		return (ELF_T_SYM);
	case SHT_FINI_ARRAY:
		return (ELF_T_ADDR);
#if	__FreeBSD_version >= 800062
	case SHT_GNU_HASH:
		return (ELF_T_GNUHASH);
#endif
	case SHT_GROUP:
		return (ELF_T_WORD);
	case SHT_HASH:
		return (ELF_T_WORD);
	case SHT_INIT_ARRAY:
		return (ELF_T_ADDR);
	case SHT_NOBITS:
		return (ELF_T_BYTE);
	case SHT_NOTE:
		return (ELF_T_NOTE);
	case SHT_PREINIT_ARRAY:
		return (ELF_T_ADDR);
	case SHT_PROGBITS:
		return (ELF_T_BYTE);
	case SHT_REL:
		return (ELF_T_REL);
	case SHT_RELA:
		return (ELF_T_RELA);
	case SHT_STRTAB:
		return (ELF_T_BYTE);
	case SHT_SYMTAB:
		return (ELF_T_SYM);
	case SHT_SYMTAB_SHNDX:
		return (ELF_T_WORD);
#if	__FreeBSD_version >= 700025
	case SHT_GNU_verdef:	/* == SHT_SUNW_verdef */
		return (ELF_T_VDEF);
	case SHT_GNU_verneed:	/* == SHT_SUNW_verneed */
		return (ELF_T_VNEED);
	case SHT_GNU_versym:	/* == SHT_SUNW_versym */
		return (ELF_T_HALF);
	case SHT_SUNW_move:
		return (ELF_T_MOVE);
	case SHT_SUNW_syminfo:
		return (ELF_T_SYMINFO);
	case SHT_SUNW_dof:
		return (ELF_T_BYTE);
#endif
	case SHT_MIPS_DWARF:
		/* FALLTHROUGH */
	case SHT_MIPS_REGINFO:
		/* FALLTHROUGH */
	case SHT_MIPS_OPTIONS:
		/* FALLTHROUGH */
	case SHT_AMD64_UNWIND:	/* == SHT_IA_64_UNWIND */
		return (ELF_T_BYTE);
	default:
		return (-1);
	}
}
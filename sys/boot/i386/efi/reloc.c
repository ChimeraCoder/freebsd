
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

#include <sys/types.h>
#include <elf.h>
#include <efi.h>
#include <bootstrap.h>

#ifdef __i386__
#define ElfW_Rel	Elf32_Rel
#define	ElfW_Dyn	Elf32_Dyn
#define	ELFW_R_TYPE	ELF32_R_TYPE
#elif __amd64__
#define ElfW_Rel	Elf64_Rel
#define	ElfW_Dyn	Elf64_Dyn
#define	ELFW_R_TYPE	ELF64_R_TYPE
#endif

/*
 * A simple relocator for IA32/AMD64 EFI binaries.
 */
EFI_STATUS
_reloc(unsigned long ImageBase, ElfW_Dyn *dynamic, EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table)
{
	unsigned long relsz, relent;
	unsigned long *newaddr;
	ElfW_Rel *rel;
	ElfW_Dyn *dynp;

	/*
	 * Find the relocation address, its size and the relocation entry.
	 */
	relsz = 0;
	relent = 0;
	for (dynp = dynamic; dynp->d_tag != DT_NULL; dynp++) {
		switch (dynp->d_tag) {
		case DT_RELA:
		case DT_REL:
			rel = (ElfW_Rel *) ((unsigned long) dynp->d_un.d_ptr +
			    ImageBase);
			break;
		case DT_RELSZ:
		case DT_RELASZ:
			relsz = dynp->d_un.d_val;
			break;
		case DT_RELENT:
		case DT_RELAENT:
			relent = dynp->d_un.d_val;
			break;
		default:
			break;
		}
	}

	/*
	 * Perform the actual relocation.
	 * XXX: We are reusing code for the amd64 version of this, but
	 * we must make sure the relocation types are the same.
	 */
	CTASSERT(R_386_NONE == R_X86_64_NONE);
	CTASSERT(R_386_RELATIVE == R_X86_64_RELATIVE);
	for (; relsz > 0; relsz -= relent) {
		switch (ELFW_R_TYPE(rel->r_info)) {
		case R_386_NONE:
			/* No relocation needs be performed. */
			break;
		case R_386_RELATIVE:
			/* Address relative to the base address. */
			newaddr = (unsigned long *)(ImageBase + rel->r_offset);
			*newaddr += ImageBase;
			break;
		default:
			/* XXX: do we need other relocations ? */
			break;
		}
		rel = (ElfW_Rel *) ((caddr_t) rel + relent);
	}

	return (EFI_SUCCESS);
}
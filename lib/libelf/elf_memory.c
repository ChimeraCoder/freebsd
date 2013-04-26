
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

#include <ar.h>
#include <libelf.h>
#include <string.h>

#include "_libelf.h"

Elf *
elf_memory(char *image, size_t sz)
{
	Elf *e;

	if (LIBELF_PRIVATE(version) == EV_NONE) {
		LIBELF_SET_ERROR(SEQUENCE, 0);
		return (NULL);
	}

	if (image == NULL || sz == 0) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if ((e = _libelf_allocate_elf()) == NULL)
		return (NULL);

	e->e_cmd = ELF_C_READ;
	e->e_rawfile = image;
	e->e_rawsize = sz;

#undef	LIBELF_IS_ELF
#define	LIBELF_IS_ELF(P) ((P)[EI_MAG0] == ELFMAG0 && 		\
	(P)[EI_MAG1] == ELFMAG1 && (P)[EI_MAG2] == ELFMAG2 &&	\
	(P)[EI_MAG3] == ELFMAG3)

	if (sz > EI_NIDENT && LIBELF_IS_ELF(image)) {
		_libelf_init_elf(e, ELF_K_ELF);
		e->e_class = image[EI_CLASS];
		e->e_byteorder = image[EI_DATA];
		e->e_version = image[EI_VERSION];

		if (e->e_version > EV_CURRENT) {
			e = _libelf_release_elf(e);
			LIBELF_SET_ERROR(VERSION, 0);
			return (NULL);
		}

		if ((e->e_byteorder != ELFDATA2LSB && e->e_byteorder !=
 		    ELFDATA2MSB) || (e->e_class != ELFCLASS32 && e->e_class !=
		    ELFCLASS64)) {
			e = _libelf_release_elf(e);
			LIBELF_SET_ERROR(HEADER, 0);
			return (NULL);
		}

	} else if (sz >= SARMAG &&
	    strncmp(image, ARMAG, (size_t) SARMAG) == 0) {
		_libelf_init_elf(e, ELF_K_AR);
		e = _libelf_ar_open(e);
	} else
		_libelf_init_elf(e, ELF_K_NONE);

	return (e);
}
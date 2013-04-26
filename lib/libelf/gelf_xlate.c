
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

#include <gelf.h>
#include <libelf.h>
#include <string.h>

#include "_libelf.h"


Elf_Data *
elf32_xlatetof(Elf_Data *dst, const Elf_Data *src, unsigned int encoding)
{
	return _libelf_xlate(dst, src, encoding, ELFCLASS32, ELF_TOFILE);
}

Elf_Data *
elf64_xlatetof(Elf_Data *dst, const Elf_Data *src, unsigned int encoding)
{
	return _libelf_xlate(dst, src, encoding, ELFCLASS64, ELF_TOFILE);
}

Elf_Data *
elf32_xlatetom(Elf_Data *dst, const Elf_Data *src, unsigned int encoding)
{
	return _libelf_xlate(dst, src, encoding, ELFCLASS32, ELF_TOMEMORY);
}

Elf_Data *
elf64_xlatetom(Elf_Data *dst, const Elf_Data *src, unsigned int encoding)
{
	return _libelf_xlate(dst, src, encoding, ELFCLASS64, ELF_TOMEMORY);
}

Elf_Data *
gelf_xlatetom(Elf *e, Elf_Data *dst, const Elf_Data *src, unsigned int encoding)
{
	if (e != NULL)
		return (_libelf_xlate(dst, src, encoding, e->e_class,
		    ELF_TOMEMORY));
	LIBELF_SET_ERROR(ARGUMENT, 0);
	return (NULL);
}

Elf_Data *
gelf_xlatetof(Elf *e, Elf_Data *dst, const Elf_Data *src, unsigned int encoding)
{
	if (e != NULL)
		return (_libelf_xlate(dst, src, encoding, e->e_class,
		    ELF_TOFILE));
	LIBELF_SET_ERROR(ARGUMENT, 0);
	return (NULL);
}
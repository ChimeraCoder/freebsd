
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

#include "_libelf.h"

size_t
elf32_fsize(Elf_Type t, size_t c, unsigned int v)
{
	return (_libelf_fsize(t, ELFCLASS32, v, c));
}

size_t
elf64_fsize(Elf_Type t, size_t c, unsigned int v)
{
	return (_libelf_fsize(t, ELFCLASS64, v, c));
}

size_t
gelf_fsize(Elf *e, Elf_Type t, size_t c, unsigned int v)
{

	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (e->e_class == ELFCLASS32 || e->e_class == ELFCLASS64)
		return (_libelf_fsize(t, e->e_class, v, c));

	LIBELF_SET_ERROR(ARGUMENT, 0);
	return (0);
}
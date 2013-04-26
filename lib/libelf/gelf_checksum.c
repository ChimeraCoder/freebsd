
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

long
elf32_checksum(Elf *e)
{
	return (_libelf_checksum(e, ELFCLASS32));
}

long
elf64_checksum(Elf *e)
{
	return (_libelf_checksum(e, ELFCLASS64));
}

long
gelf_checksum(Elf *e)
{
	int ec;
	if (e == NULL ||
	    ((ec = e->e_class) != ELFCLASS32 && ec != ELFCLASS64)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0L);
	}
	return (_libelf_checksum(e, ec));
}
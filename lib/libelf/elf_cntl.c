
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

#include "_libelf.h"

int
elf_cntl(Elf *e, Elf_Cmd c)
{
	if (e == NULL ||
	    (c != ELF_C_FDDONE && c != ELF_C_FDREAD)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (-1);
	}

	if (e->e_parent) {
		LIBELF_SET_ERROR(ARCHIVE, 0);
		return (-1);
	}

	if (c == ELF_C_FDREAD) {
		if (e->e_cmd == ELF_C_WRITE) {
			LIBELF_SET_ERROR(MODE, 0);
			return (-1);
		}
		else
			return (0);
	}

	e->e_fd = -1;
	return 0;
}
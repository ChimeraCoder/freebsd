
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

char *
elf_rawfile(Elf *e, size_t *sz)
{
	char *ptr;
	size_t size;

	size = e ? e->e_rawsize : 0;
	ptr = NULL;

	if (e == NULL)
		LIBELF_SET_ERROR(ARGUMENT, 0);
	else if ((ptr = e->e_rawfile) == NULL && e->e_cmd == ELF_C_WRITE)
		LIBELF_SET_ERROR(SEQUENCE, 0);

	if (sz)
		*sz = size;

	return (ptr);
}
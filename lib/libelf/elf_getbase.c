
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

off_t
elf_getbase(Elf *e)
{
	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return ((off_t) -1);
	}

	if (e->e_parent == NULL)
		return ((off_t) 0);

	return ((off_t) ((uintptr_t) e->e_rawfile -
	    (uintptr_t) e->e_parent->e_rawfile));
}
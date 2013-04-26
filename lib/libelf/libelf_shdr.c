
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

void *
_libelf_getshdr(Elf_Scn *s, int ec)
{
	Elf *e;

	if (s == NULL || (e = s->s_elf) == NULL ||
	    e->e_kind != ELF_K_ELF) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASSNONE)
		ec = e->e_class;

	if (ec != e->e_class) {
		LIBELF_SET_ERROR(CLASS, 0);
		return (NULL);
	}

	return ((void *) &s->s_shdr);
}
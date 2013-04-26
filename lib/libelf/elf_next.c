
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
#include <assert.h>
#include <libelf.h>

#include "_libelf.h"

Elf_Cmd
elf_next(Elf *e)
{
	off_t next;
	Elf *parent;

	if (e == NULL)
		return (ELF_C_NULL);

	 if ((parent = e->e_parent) == NULL) {
		 LIBELF_SET_ERROR(ARGUMENT, 0);
		 return (ELF_C_NULL);
	 }

	assert (parent->e_kind == ELF_K_AR);
	assert (parent->e_cmd == ELF_C_READ);
	assert((uintptr_t) e->e_rawfile % 2 == 0);
	assert(e->e_rawfile > parent->e_rawfile);

	next = e->e_rawfile - parent->e_rawfile + e->e_rawsize;
	next = (next + 1) & ~1;	/* round up to an even boundary */

	parent->e_u.e_ar.e_next = (next >= (off_t) parent->e_rawsize) ? (off_t) 0 : next;

	return (ELF_C_READ);
}
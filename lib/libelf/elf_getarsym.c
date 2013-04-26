
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

Elf_Arsym *
elf_getarsym(Elf *ar, size_t *ptr)
{
	size_t n;
	Elf_Arsym *symtab;

	n = 0;
	symtab = NULL;

	if (ar == NULL || ar->e_kind != ELF_K_AR)
		LIBELF_SET_ERROR(ARGUMENT, 0);
	else if ((symtab = ar->e_u.e_ar.e_symtab) != NULL)
		n = ar->e_u.e_ar.e_symtabsz;
	else if (ar->e_u.e_ar.e_rawsymtab)
		symtab = _libelf_ar_process_symtab(ar, &n);
	else
		LIBELF_SET_ERROR(ARCHIVE, 0);

	if (ptr)
		*ptr = n;
	return (symtab);
}

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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/types.h>
#include <string.h>

#include "symbol.h"

int
ignore_symbol(GElf_Sym *sym, const char *name)
{
	uchar_t type = GELF_ST_TYPE(sym->st_info);

	/*
	 * As an optimization, we do not output function or data object
	 * records for undefined or anonymous symbols.
	 */
	if (sym->st_shndx == SHN_UNDEF || sym->st_name == 0)
		return (1);

	/*
	 * _START_ and _END_ are added to the symbol table by the
	 * linker, and will never have associated type information.
	 */
	if (strcmp(name, "_START_") == 0 || strcmp(name, "_END_") == 0)
		return (1);

	/*
	 * Do not output records for absolute-valued object symbols
	 * that have value zero.  The compiler insists on generating
	 * things like this for __fsr_init_value settings, etc.
	 */
	if (type == STT_OBJECT && sym->st_shndx == SHN_ABS &&
	    sym->st_value == 0)
		return (1);
	return (0);
}
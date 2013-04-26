
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

#include "_libdwarf.h"

int
dwarf_next_cu_header(Dwarf_Debug dbg, Dwarf_Unsigned *cu_header_length,
    Dwarf_Half *cu_version, Dwarf_Unsigned *cu_abbrev_offset,
    Dwarf_Half *cu_pointer_size, Dwarf_Unsigned *cu_next_offset, Dwarf_Error *error)
{
	Dwarf_CU next;

	if (error == NULL)
		return DWARF_E_ERROR;

	if (dbg == NULL || cu_header_length == NULL || cu_version == NULL ||
	    cu_abbrev_offset == NULL || cu_pointer_size == NULL ||
	    cu_next_offset == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return DWARF_E_ERROR;
	}

	if (dbg->dbg_cu_current == NULL)
		dbg->dbg_cu_current = STAILQ_FIRST(&dbg->dbg_cu);
	else if ((next = STAILQ_NEXT(dbg->dbg_cu_current, cu_next)) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return DWARF_E_NO_ENTRY;
	} else
		dbg->dbg_cu_current = next;

	if (dbg->dbg_cu_current == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return DWARF_E_NO_ENTRY;
	}

	*cu_header_length	= dbg->dbg_cu_current->cu_header_length;
	*cu_version 		= dbg->dbg_cu_current->cu_version;
	*cu_abbrev_offset	= dbg->dbg_cu_current->cu_abbrev_offset;
	*cu_pointer_size	= dbg->dbg_cu_current->cu_pointer_size;
	*cu_next_offset		= dbg->dbg_cu_current->cu_next_offset;

	return DWARF_E_NONE;
}
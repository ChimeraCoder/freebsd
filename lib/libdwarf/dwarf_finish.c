
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

#include <stdlib.h>
#include "_libdwarf.h"

int
dwarf_finish(Dwarf_Debug *dbgp, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;
	Dwarf_Abbrev tab;
	Dwarf_Attribute at;
	Dwarf_Attribute tat;
	Dwarf_AttrValue av;
	Dwarf_AttrValue tav;
	Dwarf_CU cu;
	Dwarf_CU tcu;
	Dwarf_Debug dbg;
	Dwarf_Die die;
	Dwarf_Die tdie;

	if (error == NULL)
		/* Can only return a generic error. */
		return DWARF_E_ERROR;

	if (dbgp == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return DWARF_E_ERROR;
	}

	if ((dbg = *dbgp) == NULL)
		return DWARF_E_NONE;

	/* Free entries in the compilation unit list. */
	STAILQ_FOREACH_SAFE(cu, &dbg->dbg_cu, cu_next, tcu) {
		/* Free entries in the die list */
		STAILQ_FOREACH_SAFE(die, &cu->cu_die, die_next, tdie) {
			/* Free entries in the attribute value list */
			STAILQ_FOREACH_SAFE(av, &die->die_attrval, av_next, tav) {
				STAILQ_REMOVE(&die->die_attrval, av, _Dwarf_AttrValue, av_next);
				free(av);
			}

			STAILQ_REMOVE(&cu->cu_die, die, _Dwarf_Die, die_next);
			free(die);
		}

		/* Free entries in the abbrev list */
		STAILQ_FOREACH_SAFE(ab, &cu->cu_abbrev, a_next, tab) {
			/* Free entries in the attribute list */
			STAILQ_FOREACH_SAFE(at, &ab->a_attrib, at_next, tat) {
				STAILQ_REMOVE(&ab->a_attrib, at, _Dwarf_Attribute, at_next);
				free(at);
			}

			STAILQ_REMOVE(&cu->cu_abbrev, ab, _Dwarf_Abbrev, a_next);
			free(ab);
		}

		STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
		free(cu);
	}

	if (dbg->dbg_elf_close)
		/* Free resources associated with the ELF file. */
		elf_end(dbg->dbg_elf);

	free(dbg);

	*dbgp = NULL;

	return DWARF_E_NONE;
}
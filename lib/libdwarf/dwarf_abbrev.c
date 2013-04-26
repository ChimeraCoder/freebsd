
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
dwarf_abbrev_add(Dwarf_CU cu, uint64_t entry, uint64_t tag, uint8_t children, Dwarf_Abbrev *ap, Dwarf_Error *error)
{
	Dwarf_Abbrev a;
	int ret = DWARF_E_NONE;

	if ((a = malloc(sizeof(struct _Dwarf_Abbrev))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return DWARF_E_MEMORY;
	}

	/* Initialise the abbrev structure. */
	a->a_entry	= entry;
	a->a_tag	= tag;
	a->a_children	= children;

	/* Initialise the list of attributes. */
	STAILQ_INIT(&a->a_attrib);

	/* Add the abbrev to the list in the compilation unit. */
	STAILQ_INSERT_TAIL(&cu->cu_abbrev, a, a_next);

	if (ap != NULL)
		*ap = a;

	return ret;
}

Dwarf_Abbrev
dwarf_abbrev_find(Dwarf_CU cu, uint64_t entry)
{
	Dwarf_Abbrev a = NULL;

	STAILQ_FOREACH(a, &cu->cu_abbrev, a_next) {
		if (a->a_entry == entry)
			break;
	}

	return a;
}
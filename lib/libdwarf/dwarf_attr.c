
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
#include <string.h>
#include "_libdwarf.h"

int
dwarf_attr(Dwarf_Die die, Dwarf_Half attr, Dwarf_Attribute *atp, Dwarf_Error *err)
{
	Dwarf_Attribute at;
	Dwarf_Abbrev	a;
	int ret = DWARF_E_NONE;

	if (err == NULL)
		return DWARF_E_ERROR;

	if (die == NULL || atp == NULL || (a = die->die_a) == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	STAILQ_FOREACH(at, &a->a_attrib, at_next)
		if (at->at_attrib == attr)
			break;

	*atp = at;

	if (at == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_NO_ENTRY);
		ret = DWARF_E_NO_ENTRY;
	}

	return ret;
}

int
dwarf_attr_add(Dwarf_Abbrev a, uint64_t attr, uint64_t form, Dwarf_Attribute *atp, Dwarf_Error *error)
{
	Dwarf_Attribute at;
	int ret = DWARF_E_NONE;

	if (error == NULL)
		return DWARF_E_ERROR;

	if (a == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	if ((at = malloc(sizeof(struct _Dwarf_Attribute))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return DWARF_E_MEMORY;
	}

	/* Initialise the attribute structure. */
	at->at_attrib	= attr;
	at->at_form	= form;

	/* Add the attribute to the list in the abbrev. */
	STAILQ_INSERT_TAIL(&a->a_attrib, at, at_next);

	if (atp != NULL)
		*atp = at;

	return ret;
}
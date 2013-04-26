
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
dwarf_die_add(Dwarf_CU cu, int level, uint64_t offset, uint64_t abnum, Dwarf_Abbrev a, Dwarf_Die *diep, Dwarf_Error *err)
{
	Dwarf_Die die;
	uint64_t key;
	int ret = DWARF_E_NONE;

	if (err == NULL)
		return DWARF_E_ERROR;

	if (cu == NULL || a == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	if ((die = malloc(sizeof(struct _Dwarf_Die))) == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_MEMORY);
		return DWARF_E_MEMORY;
	}

	/* Initialise the abbrev structure. */
	die->die_level	= level;
	die->die_offset	= offset;
	die->die_abnum	= abnum;
	die->die_a	= a;
	die->die_cu	= cu;
	die->die_name	= "";

	/* Initialise the list of attribute values. */
	STAILQ_INIT(&die->die_attrval);

	/* Add the die to the list in the compilation unit. */
	STAILQ_INSERT_TAIL(&cu->cu_die, die, die_next);

	/* Add the die to the hash table in the compilation unit. */
	key = offset % DWARF_DIE_HASH_SIZE;
	STAILQ_INSERT_TAIL(&cu->cu_die_hash[key], die, die_hash);

	if (diep != NULL)
		*diep = die;

	return ret;
}

int
dwarf_dieoffset(Dwarf_Die die, Dwarf_Off *ret_offset, Dwarf_Error *err __unused)
{
	*ret_offset = die->die_offset;

	return DWARF_E_NONE;
}

int
dwarf_child(Dwarf_Die die, Dwarf_Die *ret_die, Dwarf_Error *err)
{
	Dwarf_Die next;
	int ret = DWARF_E_NONE;

	if (err == NULL)
		return DWARF_E_ERROR;

	if (die == NULL || ret_die == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	if ((next = STAILQ_NEXT(die, die_next)) == NULL ||
	    next->die_level != die->die_level + 1) {
		*ret_die = NULL;
		DWARF_SET_ERROR(err, DWARF_E_NO_ENTRY);
		ret = DWARF_E_NO_ENTRY;
	} else
		*ret_die = next;

	return ret;
}

int
dwarf_tag(Dwarf_Die die, Dwarf_Half *tag, Dwarf_Error *err)
{
	Dwarf_Abbrev a;

	if (err == NULL)
		return DWARF_E_ERROR;

	if (die == NULL || tag == NULL || (a = die->die_a) == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	*tag = a->a_tag;

	return DWARF_E_NONE;
}

int
dwarf_siblingof(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Die *caller_ret_die, Dwarf_Error *err)
{
	Dwarf_Die next;
	Dwarf_CU cu;
	int ret = DWARF_E_NONE;

	if (err == NULL)
		return DWARF_E_ERROR;

	if (dbg == NULL || caller_ret_die == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	if ((cu = dbg->dbg_cu_current) == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_CU_CURRENT);
		return DWARF_E_CU_CURRENT;
	}

	if (die == NULL) {
		*caller_ret_die = STAILQ_FIRST(&cu->cu_die);

		if (*caller_ret_die == NULL) {
			DWARF_SET_ERROR(err, DWARF_E_NO_ENTRY);
			ret = DWARF_E_NO_ENTRY;
		}
	} else {
		next = die;
		while ((next = STAILQ_NEXT(next, die_next)) != NULL) {
			if (next->die_level < die->die_level) {
				next = NULL;
				break;
			}
			if (next->die_level == die->die_level) {
				*caller_ret_die = next;
				break;
			}
		}

		if (next == NULL) {
			*caller_ret_die = NULL;
			DWARF_SET_ERROR(err, DWARF_E_NO_ENTRY);
			ret = DWARF_E_NO_ENTRY;
		}
	}

	return ret;
}

Dwarf_Die
dwarf_die_find(Dwarf_Die die, Dwarf_Unsigned off)
{
	Dwarf_CU cu = die->die_cu;
	Dwarf_Die die1;

	STAILQ_FOREACH(die1, &cu->cu_die, die_next) {
		if (die1->die_offset == off)
			return (die1);
	}

	return (NULL);
}
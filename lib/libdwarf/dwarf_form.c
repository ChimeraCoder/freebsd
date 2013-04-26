
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
dwarf_whatform(Dwarf_Attribute at, Dwarf_Half *return_form, Dwarf_Error *err)
{
	int ret = DWARF_E_NONE;

	if (err == NULL)
		return DWARF_E_ERROR;

	if (at == NULL || return_form == NULL) {
		DWARF_SET_ERROR(err, DWARF_E_ARGUMENT);
		return DWARF_E_ARGUMENT;
	}

	*return_form = at->at_form;

	return ret;
}
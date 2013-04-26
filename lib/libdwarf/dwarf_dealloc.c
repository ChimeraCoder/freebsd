
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

void
dwarf_dealloc(Dwarf_Debug dbg __unused, Dwarf_Ptr p __unused, Dwarf_Unsigned alloc_type __unused)
{
	/*
	 * This libdwarf implementation doesn't use this style
	 * of memory allocation. It doesn't copy things to return
	 * them to the client, so the client doesn't need to
	 * remember to free them.
	 */
	return;
}
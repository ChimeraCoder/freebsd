
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
/*
 * lib_strbuf - library string storage
 */
#include "ntp_stdlib.h"
#include "lib_strbuf.h"

/*
 * Storage declarations
 */
char lib_stringbuf[LIB_NUMBUFS][LIB_BUFLENGTH];
int lib_nextbuf;
int lib_inited = 0;

/*
 * initialization routine.  Might be needed if the code is ROMized.
 */
void
init_lib(void)
{
	lib_nextbuf = 0;
	lib_inited = 1;
}
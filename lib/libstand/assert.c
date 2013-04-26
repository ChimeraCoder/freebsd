
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

#include <assert.h>

#include "stand.h"

void
__assert(const char *func, const char *file, int line, const char *expression)
{
	if (func == NULL)
		panic("Assertion failed: (%s), file %s, line %d.\n",
		    expression, file, line);
	else
		panic(
		    "Assertion failed: (%s), function %s, file %s, line %d.\n",
		    expression, func, file, line);
}
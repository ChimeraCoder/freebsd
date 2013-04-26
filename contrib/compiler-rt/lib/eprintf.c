
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



#include "int_lib.h"
#include <stdio.h>


/*
 * __eprintf() was used in an old version of <assert.h>.
 * It can eventually go away, but it is needed when linking
 * .o files built with the old <assert.h>.
 *
 * It should never be exported from a dylib, so it is marked
 * visibility hidden.
 */
__attribute__((visibility("hidden")))
void __eprintf(const char* format, const char* assertion_expression,
				const char* line, const char* file)
{
	fprintf(stderr, format, assertion_expression, line, file);
	fflush(stderr);
	compilerrt_abort();
}

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

#include "lib.h"

/*
 * .KB_C_FN_DEFINITION_START
 * void p_memset(char *buffer, char value, int size)
 *  This global function sets memory at the pointer for the specified
 * number of bytes to value.
 * .KB_C_FN_DEFINITION_END
 */
void
p_memset(char *buffer, char value, int size)
{
	while (size--)
		*buffer++ = value;
}

/*
 * .KB_C_FN_DEFINITION_START
 * int p_memcmp(char *to, char *from, unsigned size)
 *  This global function compares data at to against data at from for
 * size bytes.  Returns 0 if the locations are equal.  size must be
 * greater than 0.
 * .KB_C_FN_DEFINITION_END
 */
int
p_memcmp(const char *to, const char *from, unsigned size)
{
	while ((--size) && (*to++ == *from++))
		continue;

	return (*to != *from);
}
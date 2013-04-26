
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

#include <uuid.h>

/*
 * uuid_is_nil() - return whether the UUID is a nil UUID.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_is_nil.htm
 */
int32_t
uuid_is_nil(const uuid_t *u, uint32_t *status)
{
	const uint32_t *p;

	if (status)
		*status = uuid_s_ok;

	if (!u)
		return (1);

	/*
	 * Pick the largest type that has equivalent alignment constraints
	 * as an UUID and use it to test if the UUID consists of all zeroes.
	 */
	p = (const uint32_t*)u;
	return ((p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0) ? 1 : 0);
}
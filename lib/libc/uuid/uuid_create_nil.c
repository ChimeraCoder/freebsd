
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

#include <strings.h>
#include <uuid.h>

/*
 * uuid_create_nil() - create a nil UUID.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_create_nil.htm
 */
void
uuid_create_nil(uuid_t *u, uint32_t *status)
{

	if (status)
		*status = uuid_s_ok;

	bzero(u, sizeof(*u));
}
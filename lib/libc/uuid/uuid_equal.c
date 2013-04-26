
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

#include <string.h>
#include <uuid.h>

/*
 * uuid_equal() - compare for equality.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_equal.htm
 */
int32_t
uuid_equal(const uuid_t *a, const uuid_t *b, uint32_t *status)
{

	if (status != NULL)
		*status = uuid_s_ok;

	/* Deal with equal or NULL pointers. */
	if (a == b)
		return (1);
	if (a == NULL)
		return (uuid_is_nil(b, NULL));
	if (b == NULL)
		return (uuid_is_nil(a, NULL));

	/* Do a byte for byte comparison. */
	return ((memcmp(a, b, sizeof(uuid_t))) ? 0 : 1);
}
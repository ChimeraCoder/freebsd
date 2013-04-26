
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

#include <stdio.h>
#include <string.h>
#include <uuid.h>

/*
 * uuid_to_string() - Convert a binary UUID into a string representation.
 * See also:
 *	http://www.opengroup.org/onlinepubs/009629399/uuid_to_string.htm
 *
 * NOTE: The references given above do not have a status code for when
 *	 the string could not be allocated. The status code has been
 *	 taken from the Hewlett-Packard implementation.
 */
void
uuid_to_string(const uuid_t *u, char **s, uint32_t *status)
{
	uuid_t nil;

	if (status != NULL)
		*status = uuid_s_ok;

	/* Why allow a NULL-pointer here? */
	if (s == 0)
		return;

	if (u == NULL) {
		u = &nil;
		uuid_create_nil(&nil, NULL);
	}

	asprintf(s, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	    u->time_low, u->time_mid, u->time_hi_and_version,
	    u->clock_seq_hi_and_reserved, u->clock_seq_low, u->node[0],
	    u->node[1], u->node[2], u->node[3], u->node[4], u->node[5]);

	if (*s == NULL && status != NULL)
		*status = uuid_s_no_memory;
}
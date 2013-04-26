
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

#include <gssapi/gssapi.h>
#include <stdlib.h>
#include <errno.h>

OM_uint32
gss_create_empty_oid_set(OM_uint32 *minor_status,
    gss_OID_set *oid_set)
{
	gss_OID_set set;

	*minor_status = 0;
	*oid_set = GSS_C_NO_OID_SET;

	set = malloc(sizeof(gss_OID_set_desc));
	if (!set) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	set->count = 0;
	set->elements = 0;
	*oid_set = set;

	return (GSS_S_COMPLETE);
}
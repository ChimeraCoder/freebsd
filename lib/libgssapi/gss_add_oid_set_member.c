
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
#include <string.h>
#include <errno.h>

OM_uint32
gss_add_oid_set_member(OM_uint32 *minor_status,
    const gss_OID member_oid,
    gss_OID_set *oid_set)
{
	OM_uint32 major_status;
	gss_OID_set set = *oid_set;
	gss_OID new_elements;
	gss_OID new_oid;
	int t;

	*minor_status = 0;

	major_status = gss_test_oid_set_member(minor_status,
	    member_oid, *oid_set, &t);
	if (major_status)
		return (major_status);
	if (t)
		return (GSS_S_COMPLETE);

	new_elements = malloc((set->count + 1) * sizeof(gss_OID_desc));
	if (!new_elements) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	new_oid = &new_elements[set->count];
	new_oid->elements = malloc(member_oid->length);
	if (!new_oid->elements) {
		free(new_elements);
		return (GSS_S_FAILURE);
	}
	new_oid->length = member_oid->length;
	memcpy(new_oid->elements, member_oid->elements, member_oid->length);

	if (set->elements) {
		memcpy(new_elements, set->elements,
		    set->count * sizeof(gss_OID_desc));
		free(set->elements);
	}
	set->elements = new_elements;
	set->count++;

	return (GSS_S_COMPLETE);
}
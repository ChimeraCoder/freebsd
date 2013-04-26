
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

#include "mech_locl.h"

/**
 * Add a oid to the oid set, function does not make a copy of the oid,
 * so the pointer to member_oid needs to be stable for the whole time
 * oid_set is used.
 *
 * If there is a duplicate member of the oid, the new member is not
 * added to to the set.
 *
 * @param minor_status minor status code.
 * @param member_oid member to add to the oid set
 * @param oid_set oid set to add the member too
 *
 * @returns a gss_error code, see gss_display_status() about printing
 *          the error code.
 *
 * @ingroup gssapi
 */

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_add_oid_set_member (OM_uint32 * minor_status,
			const gss_OID member_oid,
			gss_OID_set * oid_set)
{
    gss_OID tmp;
    size_t n;
    OM_uint32 res;
    int present;

    res = gss_test_oid_set_member(minor_status, member_oid, *oid_set, &present);
    if (res != GSS_S_COMPLETE)
	return res;

    if (present) {
	*minor_status = 0;
	return GSS_S_COMPLETE;
    }

    n = (*oid_set)->count + 1;
    tmp = realloc ((*oid_set)->elements, n * sizeof(gss_OID_desc));
    if (tmp == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }
    (*oid_set)->elements = tmp;
    (*oid_set)->count = n;
    (*oid_set)->elements[n-1] = *member_oid;
    *minor_status = 0;
    return GSS_S_COMPLETE;
}
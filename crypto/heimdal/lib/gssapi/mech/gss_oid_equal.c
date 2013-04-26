
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
 * Compare two GSS-API OIDs with each other.
 *
 * GSS_C_NO_OID matches nothing, not even it-self.
 *
 * @param a first oid to compare
 * @param b second oid to compare
 *
 * @return non-zero when both oid are the same OID, zero when they are
 *         not the same.
 *
 * @ingroup gssapi
 */

GSSAPI_LIB_FUNCTION int GSSAPI_LIB_CALL
gss_oid_equal(gss_const_OID a, gss_const_OID b)
{
    if (a == b && a != GSS_C_NO_OID)
	return 1;
    if (a == GSS_C_NO_OID || b == GSS_C_NO_OID || a->length != b->length)
	return 0;
    return memcmp(a->elements, b->elements, a->length) == 0;
}
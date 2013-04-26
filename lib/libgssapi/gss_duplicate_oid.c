
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

OM_uint32 gss_duplicate_oid(OM_uint32 *minor_status,
    const gss_OID src_oid,
    gss_OID *dest_oid_p)
{
	gss_OID dest_oid;

	*minor_status = 0;
	*dest_oid_p = GSS_C_NO_OID;

	if (src_oid == GSS_C_NO_OID)
		return (GSS_S_COMPLETE);

	dest_oid = malloc(sizeof(gss_OID_desc));
	if (!dest_oid) {
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	dest_oid->elements = malloc(src_oid->length);
	if (!dest_oid->elements) {
		free(dest_oid);
		*minor_status = ENOMEM;
		return (GSS_S_FAILURE);
	}

	memcpy(dest_oid->elements, src_oid->elements, src_oid->length);
	dest_oid->length = src_oid->length;

	*dest_oid_p = dest_oid;

	return (GSS_S_COMPLETE);
}
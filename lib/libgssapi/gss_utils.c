
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

#include "utils.h"

int
gss_oid_equal(const gss_OID oid1, const gss_OID oid2)
{

	if (oid1 == oid2)
		return (1);
	if (!oid1 || !oid2)
		return (0);
	if (oid1->length != oid2->length)
		return (0);
	if (memcmp(oid1->elements, oid2->elements, oid1->length))
		return (0);
	return (1);
}

OM_uint32
_gss_copy_oid(OM_uint32 *minor_status,
    const gss_OID from_oid, gss_OID to_oid)
{
	size_t len = from_oid->length;

	*minor_status = 0;
	to_oid->elements = malloc(len);
	if (!to_oid->elements) {
		to_oid->length = 0;
		*minor_status = ENOMEM;
		return GSS_S_FAILURE;
	}
	to_oid->length = len;
	memcpy(to_oid->elements, from_oid->elements, len);
	return (GSS_S_COMPLETE);
}

OM_uint32
_gss_free_oid(OM_uint32 *minor_status, gss_OID oid)
{

	*minor_status = 0;
	if (oid->elements) {
		free(oid->elements);
		oid->elements = NULL;
		oid->length = 0;
	}
	return (GSS_S_COMPLETE);
}

OM_uint32
_gss_copy_buffer(OM_uint32 *minor_status,
    const gss_buffer_t from_buf, gss_buffer_t to_buf)
{
	size_t len = from_buf->length;

	*minor_status = 0;
	to_buf->value = malloc(len);
	if (!to_buf->value) {
		*minor_status = ENOMEM;
		to_buf->length = 0;
		return GSS_S_FAILURE;
	}
	to_buf->length = len;
	memcpy(to_buf->value, from_buf->value, len);
	return (GSS_S_COMPLETE);
}
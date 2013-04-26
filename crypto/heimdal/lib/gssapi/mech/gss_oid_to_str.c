
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

GSSAPI_LIB_FUNCTION OM_uint32 GSSAPI_LIB_CALL
gss_oid_to_str(OM_uint32 *minor_status, gss_OID oid, gss_buffer_t oid_str)
{
    int ret;
    size_t size;
    heim_oid o;
    char *p;

    _mg_buffer_zero(oid_str);

    if (oid == GSS_C_NULL_OID)
	return GSS_S_FAILURE;

    ret = der_get_oid (oid->elements, oid->length, &o, &size);
    if (ret) {
	*minor_status = ret;
	return GSS_S_FAILURE;
    }

    ret = der_print_heim_oid(&o, ' ', &p);
    der_free_oid(&o);
    if (ret) {
	*minor_status = ret;
	return GSS_S_FAILURE;
    }

    oid_str->value = p;
    oid_str->length = strlen(p);

    *minor_status = 0;
    return GSS_S_COMPLETE;
}

GSSAPI_LIB_FUNCTION const char * GSSAPI_LIB_CALL
gss_oid_to_name(gss_const_OID oid)
{
    size_t i;

    for (i = 0; _gss_ont_mech[i].oid; i++) {
	if (gss_oid_equal(oid, _gss_ont_mech[i].oid))
	    return _gss_ont_mech[i].name;
    }
    return NULL;
}

GSSAPI_LIB_FUNCTION gss_OID GSSAPI_LIB_CALL
gss_name_to_oid(const char *name)
{
    size_t i, partial = (size_t)-1;

    for (i = 0; _gss_ont_mech[i].oid; i++) {
	if (strcasecmp(name, _gss_ont_mech[i].short_desc) == 0)
	    return _gss_ont_mech[i].oid;
	if (strncasecmp(name, _gss_ont_mech[i].short_desc, strlen(name)) == 0) {
	    if (partial != (size_t)-1)
		return NULL;
	    partial = i;
	}
    }
    if (partial != (size_t)-1)
	return _gss_ont_mech[partial].oid;
    return NULL;
}
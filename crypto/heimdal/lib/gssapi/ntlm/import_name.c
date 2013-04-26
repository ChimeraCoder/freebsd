
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

#include "ntlm.h"

OM_uint32 GSSAPI_CALLCONV
_gss_ntlm_import_name
           (OM_uint32 * minor_status,
            const gss_buffer_t input_name_buffer,
            const gss_OID input_name_type,
            gss_name_t * output_name
           )
{
    char *name, *p, *p2;
    int is_hostnamed;
    int is_username;
    ntlm_name n;

    *minor_status = 0;

    if (output_name == NULL)
	return GSS_S_CALL_INACCESSIBLE_WRITE;

    *output_name = GSS_C_NO_NAME;

    is_hostnamed = gss_oid_equal(input_name_type, GSS_C_NT_HOSTBASED_SERVICE);
    is_username = gss_oid_equal(input_name_type, GSS_C_NT_USER_NAME);

    if (!is_hostnamed && !is_username)
	return GSS_S_BAD_NAMETYPE;

    name = malloc(input_name_buffer->length + 1);
    if (name == NULL) {
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }
    memcpy(name, input_name_buffer->value, input_name_buffer->length);
    name[input_name_buffer->length] = '\0';

    /* find "domain" part of the name and uppercase it */
    p = strchr(name, '@');
    if (p == NULL) {
        free(name);
	return GSS_S_BAD_NAME;
    }
    p[0] = '\0';
    p++;
    p2 = strchr(p, '.');
    if (p2 && p2[1] != '\0') {
	if (is_hostnamed) {
	    p = p2 + 1;
	    p2 = strchr(p, '.');
	}
	if (p2)
	    *p2 = '\0';
    }
    strupr(p);

    n = calloc(1, sizeof(*n));
    if (n == NULL) {
	free(name);
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    n->user = strdup(name);
    n->domain = strdup(p);

    free(name);

    if (n->user == NULL || n->domain == NULL) {
	free(n->user);
	free(n->domain);
	free(n);
	*minor_status = ENOMEM;
	return GSS_S_FAILURE;
    }

    *output_name = (gss_name_t)n;

    return GSS_S_COMPLETE;
}
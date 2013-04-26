
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

#include "gsskrb5_locl.h"

OM_uint32 GSSAPI_CALLCONV _gsskrb5_inquire_cred_by_oid
	   (OM_uint32 * minor_status,
	    const gss_cred_id_t cred_handle,
	    const gss_OID desired_object,
	    gss_buffer_set_t *data_set)
{
    krb5_context context;
    gsskrb5_cred cred = (gsskrb5_cred)cred_handle;
    krb5_error_code ret;
    gss_buffer_desc buffer;
    char *str;

    GSSAPI_KRB5_INIT (&context);

    if (gss_oid_equal(desired_object, GSS_KRB5_COPY_CCACHE_X) == 0) {
	*minor_status = EINVAL;
	return GSS_S_FAILURE;
    }

    HEIMDAL_MUTEX_lock(&cred->cred_id_mutex);

    if (cred->ccache == NULL) {
	HEIMDAL_MUTEX_unlock(&cred->cred_id_mutex);
	*minor_status = EINVAL;
	return GSS_S_FAILURE;
    }

    ret = krb5_cc_get_full_name(context, cred->ccache, &str);
    HEIMDAL_MUTEX_unlock(&cred->cred_id_mutex);
    if (ret) {
	*minor_status = ret;
	return GSS_S_FAILURE;
    }

    buffer.value = str;
    buffer.length = strlen(str);

    ret = gss_add_buffer_set_member(minor_status, &buffer, data_set);
    if (ret != GSS_S_COMPLETE)
	_gsskrb5_clear_status ();

    free(str);

    *minor_status = 0;
    return GSS_S_COMPLETE;
}
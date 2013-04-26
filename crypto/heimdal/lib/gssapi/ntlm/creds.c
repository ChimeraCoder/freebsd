
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
_gss_ntlm_inquire_cred
           (OM_uint32 * minor_status,
            const gss_cred_id_t cred_handle,
            gss_name_t * name,
            OM_uint32 * lifetime,
            gss_cred_usage_t * cred_usage,
            gss_OID_set * mechanisms
           )
{
    OM_uint32 ret, junk;

    *minor_status = 0;

    if (cred_handle == NULL)
	return GSS_S_NO_CRED;

    if (name) {
	ntlm_name n = calloc(1, sizeof(*n));
	ntlm_cred c = (ntlm_cred)cred_handle;
	if (n) {
	    n->user = strdup(c->username);
	    n->domain = strdup(c->domain);
	}
	if (n == NULL || n->user == NULL || n->domain == NULL) {
	    if (n)
		free(n->user);
	    *minor_status = ENOMEM;
	    return GSS_S_FAILURE;
	}
	*name = (gss_name_t)n;
    }
    if (lifetime)
	*lifetime = GSS_C_INDEFINITE;
    if (cred_usage)
	*cred_usage = 0;
    if (mechanisms)
	*mechanisms = GSS_C_NO_OID_SET;

    if (cred_handle == GSS_C_NO_CREDENTIAL)
	return GSS_S_NO_CRED;

    if (mechanisms) {
        ret = gss_create_empty_oid_set(minor_status, mechanisms);
        if (ret)
	    goto out;
	ret = gss_add_oid_set_member(minor_status,
				     GSS_NTLM_MECHANISM,
				     mechanisms);
        if (ret)
	    goto out;
    }

    return GSS_S_COMPLETE;
out:
    gss_release_oid_set(&junk, mechanisms);
    return ret;
}

#ifdef HAVE_KCM
static OM_uint32
_gss_ntlm_destroy_kcm_cred(gss_cred_id_t *cred_handle)
{
    krb5_storage *request, *response;
    krb5_data response_data;
    krb5_context context;
    krb5_error_code ret;
    ntlm_cred cred;

    cred = (ntlm_cred)*cred_handle;

    ret = krb5_init_context(&context);
    if (ret)
        return ret;

    ret = krb5_kcm_storage_request(context, KCM_OP_DEL_NTLM_CRED, &request);
    if (ret)
	goto out;

    ret = krb5_store_stringz(request, cred->username);
    if (ret)
	goto out;

    ret = krb5_store_stringz(request, cred->domain);
    if (ret)
	goto out;

    ret = krb5_kcm_call(context, request, &response, &response_data);
    if (ret)
	goto out;

    krb5_storage_free(request);
    krb5_storage_free(response);
    krb5_data_free(&response_data);

 out:
    krb5_free_context(context);

    return ret;
}
#endif /* HAVE_KCM */

OM_uint32 GSSAPI_CALLCONV
_gss_ntlm_destroy_cred(OM_uint32 *minor_status,
		       gss_cred_id_t *cred_handle)
{
#ifdef HAVE_KCM
    krb5_error_code ret;
#endif

    if (cred_handle == NULL || *cred_handle == GSS_C_NO_CREDENTIAL)
	return GSS_S_COMPLETE;

#ifdef HAVE_KCM
    ret = _gss_ntlm_destroy_kcm_cred(cred_handle);
    if (ret) {
	*minor_status = ret;
	return GSS_S_FAILURE;
    }
#endif

    return _gss_ntlm_release_cred(minor_status, cred_handle);
}
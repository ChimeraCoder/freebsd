
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

void GSSAPI_CALLCONV
_gss_ntlm_iter_creds_f(OM_uint32 flags,
		       void *userctx ,
		       void (*cred_iter)(void *, gss_OID, gss_cred_id_t))
{
#ifdef HAVE_KCM
    krb5_error_code ret;
    krb5_context context = NULL;
    krb5_storage *request, *response;
    krb5_data response_data;

    ret = krb5_init_context(&context);
    if (ret)
	goto done;

    ret = krb5_kcm_storage_request(context, KCM_OP_GET_NTLM_USER_LIST, &request);
    if (ret)
	goto done;

    ret = krb5_kcm_call(context, request, &response, &response_data);
    krb5_storage_free(request);
    if (ret)
	goto done;

    while (1) {
	uint32_t morep;
	char *user = NULL, *domain = NULL;
	ntlm_cred dn;

	ret = krb5_ret_uint32(response, &morep);
	if (ret) goto out;

	if (!morep) goto out;

	ret = krb5_ret_stringz(response, &user);
	if (ret) goto out;
	ret = krb5_ret_stringz(response, &domain);
	if (ret) {
	    free(user);
	    goto out;
	}

	dn = calloc(1, sizeof(*dn));
	if (dn == NULL) {
	    free(user);
	    free(domain);
	    goto out;
	}
	dn->username = user;
	dn->domain = domain;

	cred_iter(userctx, GSS_NTLM_MECHANISM, (gss_cred_id_t)dn);
    }
 out:
    krb5_storage_free(response);
    krb5_data_free(&response_data);
 done:
    if (context)
	krb5_free_context(context);
#endif /* HAVE_KCM */
    (*cred_iter)(userctx, NULL, NULL);
}
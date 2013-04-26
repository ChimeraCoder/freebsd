
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

OM_uint32 GSSAPI_CALLCONV _gsskrb5_release_cred
           (OM_uint32 * minor_status,
            gss_cred_id_t * cred_handle
           )
{
    krb5_context context;
    gsskrb5_cred cred;
    OM_uint32 junk;

    *minor_status = 0;

    if (*cred_handle == NULL)
        return GSS_S_COMPLETE;

    cred = (gsskrb5_cred)*cred_handle;
    *cred_handle = GSS_C_NO_CREDENTIAL;

    GSSAPI_KRB5_INIT (&context);

    HEIMDAL_MUTEX_lock(&cred->cred_id_mutex);

    if (cred->principal != NULL)
        krb5_free_principal(context, cred->principal);
    if (cred->keytab != NULL)
	krb5_kt_close(context, cred->keytab);
    if (cred->ccache != NULL) {
	if (cred->cred_flags & GSS_CF_DESTROY_CRED_ON_RELEASE)
	    krb5_cc_destroy(context, cred->ccache);
	else
	    krb5_cc_close(context, cred->ccache);
    }
    gss_release_oid_set(&junk, &cred->mechanisms);
    if (cred->enctypes)
	free(cred->enctypes);
    HEIMDAL_MUTEX_unlock(&cred->cred_id_mutex);
    HEIMDAL_MUTEX_destroy(&cred->cred_id_mutex);
    memset(cred, 0, sizeof(*cred));
    free(cred);
    return GSS_S_COMPLETE;
}
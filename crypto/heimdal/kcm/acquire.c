
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

#include "kcm_locl.h"

/*
 * Get a new ticket using a keytab/cached key and swap it into
 * an existing redentials cache
 */

krb5_error_code
kcm_ccache_acquire(krb5_context context,
		   kcm_ccache ccache,
		   krb5_creds **credp)
{
    krb5_error_code ret = 0;
    krb5_creds cred;
    krb5_const_realm realm;
    krb5_get_init_creds_opt *opt = NULL;
    krb5_ccache_data ccdata;
    char *in_tkt_service = NULL;

    memset(&cred, 0, sizeof(cred));

    KCM_ASSERT_VALID(ccache);

    /* We need a cached key or keytab to acquire credentials */
    if (ccache->flags & KCM_FLAGS_USE_CACHED_KEY) {
	if (ccache->key.keyblock.keyvalue.length == 0)
	    krb5_abortx(context,
			"kcm_ccache_acquire: KCM_FLAGS_USE_CACHED_KEY without key");
    } else if (ccache->flags & KCM_FLAGS_USE_KEYTAB) {
	if (ccache->key.keytab == NULL)
	    krb5_abortx(context,
			"kcm_ccache_acquire: KCM_FLAGS_USE_KEYTAB without keytab");
    } else {
	kcm_log(0, "Cannot acquire initial credentials for cache %s without key",
		ccache->name);
	return KRB5_FCC_INTERNAL;
    }

    HEIMDAL_MUTEX_lock(&ccache->mutex);

    /* Fake up an internal ccache */
    kcm_internal_ccache(context, ccache, &ccdata);

    /* Now, actually acquire the creds */
    if (ccache->server != NULL) {
	ret = krb5_unparse_name(context, ccache->server, &in_tkt_service);
	if (ret) {
	    kcm_log(0, "Failed to unparse service principal name for cache %s: %s",
		    ccache->name, krb5_get_err_text(context, ret));
	    return ret;
	}
    }

    realm = krb5_principal_get_realm(context, ccache->client);

    ret = krb5_get_init_creds_opt_alloc(context, &opt);
    if (ret)
	goto out;
    krb5_get_init_creds_opt_set_default_flags(context, "kcm", realm, opt);
    if (ccache->tkt_life != 0)
	krb5_get_init_creds_opt_set_tkt_life(opt, ccache->tkt_life);
    if (ccache->renew_life != 0)
	krb5_get_init_creds_opt_set_renew_life(opt, ccache->renew_life);

    if (ccache->flags & KCM_FLAGS_USE_CACHED_KEY) {
	ret = krb5_get_init_creds_keyblock(context,
					   &cred,
					   ccache->client,
					   &ccache->key.keyblock,
					   0,
					   in_tkt_service,
					   opt);
    } else {
	/* loosely based on lib/krb5/init_creds_pw.c */
	ret = krb5_get_init_creds_keytab(context,
					 &cred,
					 ccache->client,
					 ccache->key.keytab,
					 0,
					 in_tkt_service,
					 opt);
    }

    if (ret) {
	kcm_log(0, "Failed to acquire credentials for cache %s: %s",
		ccache->name, krb5_get_err_text(context, ret));
	if (in_tkt_service != NULL)
	    free(in_tkt_service);
	goto out;
    }

    if (in_tkt_service != NULL)
	free(in_tkt_service);

    /* Swap them in */
    kcm_ccache_remove_creds_internal(context, ccache);

    ret = kcm_ccache_store_cred_internal(context, ccache, &cred, 0, credp);
    if (ret) {
	kcm_log(0, "Failed to store credentials for cache %s: %s",
		ccache->name, krb5_get_err_text(context, ret));
	krb5_free_cred_contents(context, &cred);
	goto out;
    }

out:
    if (opt)
	krb5_get_init_creds_opt_free(context, opt);

    HEIMDAL_MUTEX_unlock(&ccache->mutex);

    return ret;
}
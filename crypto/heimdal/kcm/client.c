
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
#include <pwd.h>

krb5_error_code
kcm_ccache_resolve_client(krb5_context context,
			  kcm_client *client,
			  kcm_operation opcode,
			  const char *name,
			  kcm_ccache *ccache)
{
    krb5_error_code ret;

    ret = kcm_ccache_resolve(context, name, ccache);
    if (ret) {
	kcm_log(1, "Failed to resolve cache %s: %s",
		name, krb5_get_err_text(context, ret));
	return ret;
    }

    ret = kcm_access(context, client, opcode, *ccache);
    if (ret) {
	ret = KRB5_FCC_NOFILE; /* don't disclose */
	kcm_release_ccache(context, *ccache);
    }

    return ret;
}

krb5_error_code
kcm_ccache_destroy_client(krb5_context context,
			  kcm_client *client,
			  const char *name)
{
    krb5_error_code ret;
    kcm_ccache ccache;

    ret = kcm_ccache_resolve(context, name, &ccache);
    if (ret) {
	kcm_log(1, "Failed to resolve cache %s: %s",
		name, krb5_get_err_text(context, ret));
	return ret;
    }

    ret = kcm_access(context, client, KCM_OP_DESTROY, ccache);
    kcm_cleanup_events(context, ccache);
    kcm_release_ccache(context, ccache);
    if (ret)
	return ret;

    return kcm_ccache_destroy(context, name);
}

krb5_error_code
kcm_ccache_new_client(krb5_context context,
		      kcm_client *client,
		      const char *name,
		      kcm_ccache *ccache_p)
{
    krb5_error_code ret;
    kcm_ccache ccache;

    /* We insist the ccache name starts with UID or UID: */
    if (name_constraints != 0) {
	char prefix[64];
	size_t prefix_len;
	int bad = 1;

	snprintf(prefix, sizeof(prefix), "%ld:", (long)client->uid);
	prefix_len = strlen(prefix);

	if (strncmp(name, prefix, prefix_len) == 0)
	    bad = 0;
	else {
	    prefix[prefix_len - 1] = '\0';
	    if (strcmp(name, prefix) == 0)
		bad = 0;
	}

	/* Allow root to create badly-named ccaches */
	if (bad && !CLIENT_IS_ROOT(client))
	    return KRB5_CC_BADNAME;
    }

    ret = kcm_ccache_resolve(context, name, &ccache);
    if (ret == 0) {
	if ((ccache->uid != client->uid ||
	     ccache->gid != client->gid) && !CLIENT_IS_ROOT(client))
	    return KRB5_FCC_PERM;
    } else if (ret != KRB5_FCC_NOFILE && !(CLIENT_IS_ROOT(client) && ret == KRB5_FCC_PERM)) {
		return ret;
    }

    if (ret == KRB5_FCC_NOFILE) {
	ret = kcm_ccache_new(context, name, &ccache);
	if (ret) {
	    kcm_log(1, "Failed to initialize cache %s: %s",
		    name, krb5_get_err_text(context, ret));
	    return ret;
	}

	/* bind to current client */
	ccache->uid = client->uid;
	ccache->gid = client->gid;
	ccache->session = client->session;
    } else {
	ret = kcm_zero_ccache_data(context, ccache);
	if (ret) {
	    kcm_log(1, "Failed to empty cache %s: %s",
		    name, krb5_get_err_text(context, ret));
	    kcm_release_ccache(context, ccache);
	    return ret;
	}
	kcm_cleanup_events(context, ccache);
    }

    ret = kcm_access(context, client, KCM_OP_INITIALIZE, ccache);
    if (ret) {
	kcm_release_ccache(context, ccache);
	kcm_ccache_destroy(context, name);
	return ret;
    }

    /*
     * Finally, if the user is root and the cache was created under
     * another user's name, chown the cache to that user and their
     * default gid.
     */
    if (CLIENT_IS_ROOT(client)) {
	unsigned long uid;
	int matches = sscanf(name,"%ld:",&uid);
	if (matches == 0)
	    matches = sscanf(name,"%ld",&uid);
	if (matches == 1) {
	    struct passwd *pwd = getpwuid(uid);
	    if (pwd != NULL) {
		gid_t gid = pwd->pw_gid;
		kcm_chown(context, client, ccache, uid, gid);
	    }
	}
    }

    *ccache_p = ccache;
    return 0;
}
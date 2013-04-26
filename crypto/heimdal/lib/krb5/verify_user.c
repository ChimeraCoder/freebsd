
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

#include "krb5_locl.h"

static krb5_error_code
verify_common (krb5_context context,
	       krb5_principal principal,
	       krb5_ccache ccache,
	       krb5_keytab keytab,
	       krb5_boolean secure,
	       const char *service,
	       krb5_creds cred)
{
    krb5_error_code ret;
    krb5_principal server;
    krb5_verify_init_creds_opt vopt;
    krb5_ccache id;

    ret = krb5_sname_to_principal (context, NULL, service, KRB5_NT_SRV_HST,
				   &server);
    if(ret)
	return ret;

    krb5_verify_init_creds_opt_init(&vopt);
    krb5_verify_init_creds_opt_set_ap_req_nofail(&vopt, secure);

    ret = krb5_verify_init_creds(context,
				 &cred,
				 server,
				 keytab,
				 NULL,
				 &vopt);
    krb5_free_principal(context, server);
    if(ret)
	return ret;
    if(ccache == NULL)
	ret = krb5_cc_default (context, &id);
    else
	id = ccache;
    if(ret == 0){
	ret = krb5_cc_initialize(context, id, principal);
	if(ret == 0){
	    ret = krb5_cc_store_cred(context, id, &cred);
	}
	if(ccache == NULL)
	    krb5_cc_close(context, id);
    }
    krb5_free_cred_contents(context, &cred);
    return ret;
}

/*
 * Verify user `principal' with `password'.
 *
 * If `secure', also verify against local service key for `service'.
 *
 * As a side effect, fresh tickets are obtained and stored in `ccache'.
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_init(krb5_verify_opt *opt)
{
    memset(opt, 0, sizeof(*opt));
    opt->secure = TRUE;
    opt->service = "host";
}

KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_verify_opt_alloc(krb5_context context, krb5_verify_opt **opt)
{
    *opt = calloc(1, sizeof(**opt));
    if ((*opt) == NULL) {
	krb5_set_error_message(context, ENOMEM,
			       N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    krb5_verify_opt_init(*opt);
    return 0;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_free(krb5_verify_opt *opt)
{
    free(opt);
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_set_ccache(krb5_verify_opt *opt, krb5_ccache ccache)
{
    opt->ccache = ccache;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_set_keytab(krb5_verify_opt *opt, krb5_keytab keytab)
{
    opt->keytab = keytab;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_set_secure(krb5_verify_opt *opt, krb5_boolean secure)
{
    opt->secure = secure;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_set_service(krb5_verify_opt *opt, const char *service)
{
    opt->service = service;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_verify_opt_set_flags(krb5_verify_opt *opt, unsigned int flags)
{
    opt->flags |= flags;
}

static krb5_error_code
verify_user_opt_int(krb5_context context,
		    krb5_principal principal,
		    const char *password,
		    krb5_verify_opt *vopt)

{
    krb5_error_code ret;
    krb5_get_init_creds_opt *opt;
    krb5_creds cred;

    ret = krb5_get_init_creds_opt_alloc (context, &opt);
    if (ret)
	return ret;
    krb5_get_init_creds_opt_set_default_flags(context, NULL,
					      krb5_principal_get_realm(context, principal),
					      opt);
    ret = krb5_get_init_creds_password (context,
					&cred,
					principal,
					password,
					krb5_prompter_posix,
					NULL,
					0,
					NULL,
					opt);
    krb5_get_init_creds_opt_free(context, opt);
    if(ret)
	return ret;
#define OPT(V, D) ((vopt && (vopt->V)) ? (vopt->V) : (D))
    return verify_common (context, principal, OPT(ccache, NULL),
			  OPT(keytab, NULL), vopt ? vopt->secure : TRUE,
			  OPT(service, "host"), cred);
#undef OPT
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_verify_user_opt(krb5_context context,
		     krb5_principal principal,
		     const char *password,
		     krb5_verify_opt *opt)
{
    krb5_error_code ret;

    if(opt && (opt->flags & KRB5_VERIFY_LREALMS)) {
	krb5_realm *realms, *r;
	ret = krb5_get_default_realms (context, &realms);
	if (ret)
	    return ret;
	ret = KRB5_CONFIG_NODEFREALM;

	for (r = realms; *r != NULL && ret != 0; ++r) {
	    ret = krb5_principal_set_realm(context, principal, *r);
	    if (ret) {
		krb5_free_host_realm (context, realms);
		return ret;
	    }

	    ret = verify_user_opt_int(context, principal, password, opt);
	}
	krb5_free_host_realm (context, realms);
	if(ret)
	    return ret;
    } else
	ret = verify_user_opt_int(context, principal, password, opt);
    return ret;
}

/* compat function that calls above */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_verify_user(krb5_context context,
		 krb5_principal principal,
		 krb5_ccache ccache,
		 const char *password,
		 krb5_boolean secure,
		 const char *service)
{
    krb5_verify_opt opt;

    krb5_verify_opt_init(&opt);

    krb5_verify_opt_set_ccache(&opt, ccache);
    krb5_verify_opt_set_secure(&opt, secure);
    krb5_verify_opt_set_service(&opt, service);

    return krb5_verify_user_opt(context, principal, password, &opt);
}

/*
 * A variant of `krb5_verify_user'.  The realm of `principal' is
 * ignored and all the local realms are tried.
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_verify_user_lrealm(krb5_context context,
			krb5_principal principal,
			krb5_ccache ccache,
			const char *password,
			krb5_boolean secure,
			const char *service)
{
    krb5_verify_opt opt;

    krb5_verify_opt_init(&opt);

    krb5_verify_opt_set_ccache(&opt, ccache);
    krb5_verify_opt_set_secure(&opt, secure);
    krb5_verify_opt_set_service(&opt, service);
    krb5_verify_opt_set_flags(&opt, KRB5_VERIFY_LREALMS);

    return krb5_verify_user_opt(context, principal, password, &opt);
}

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

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_mk_req_exact(krb5_context context,
		  krb5_auth_context *auth_context,
		  const krb5_flags ap_req_options,
		  const krb5_principal server,
		  krb5_data *in_data,
		  krb5_ccache ccache,
		  krb5_data *outbuf)
{
    krb5_error_code ret;
    krb5_creds this_cred, *cred;

    memset(&this_cred, 0, sizeof(this_cred));

    ret = krb5_cc_get_principal(context, ccache, &this_cred.client);

    if(ret)
	return ret;

    ret = krb5_copy_principal (context, server, &this_cred.server);
    if (ret) {
	krb5_free_cred_contents (context, &this_cred);
	return ret;
    }

    this_cred.times.endtime = 0;
    if (auth_context && *auth_context && (*auth_context)->keytype)
	this_cred.session.keytype = (*auth_context)->keytype;

    ret = krb5_get_credentials (context, 0, ccache, &this_cred, &cred);
    krb5_free_cred_contents(context, &this_cred);
    if (ret)
	return ret;

    ret = krb5_mk_req_extended (context,
				auth_context,
				ap_req_options,
				in_data,
				cred,
				outbuf);
    krb5_free_creds(context, cred);
    return ret;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_mk_req(krb5_context context,
	    krb5_auth_context *auth_context,
	    const krb5_flags ap_req_options,
	    const char *service,
	    const char *hostname,
	    krb5_data *in_data,
	    krb5_ccache ccache,
	    krb5_data *outbuf)
{
    krb5_error_code ret;
    char **realms;
    char *real_hostname;
    krb5_principal server;

    ret = krb5_expand_hostname_realms (context, hostname,
				       &real_hostname, &realms);
    if (ret)
	return ret;

    ret = krb5_build_principal (context, &server,
				strlen(*realms),
				*realms,
				service,
				real_hostname,
				NULL);
    free (real_hostname);
    krb5_free_host_realm (context, realms);
    if (ret)
	return ret;
    ret = krb5_mk_req_exact (context, auth_context, ap_req_options,
			     server, in_data, ccache, outbuf);
    krb5_free_principal (context, server);
    return ret;
}
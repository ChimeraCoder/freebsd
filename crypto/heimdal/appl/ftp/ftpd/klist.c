
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

#include "ftpd_locl.h"

#ifdef KRB5

static int
print_cred(krb5_context context, krb5_creds *cred)
{
    char t1[128], t2[128], *str;
    krb5_error_code ret;
    krb5_timestamp sec;

    krb5_timeofday (context, &sec);

    if(cred->times.starttime)
	krb5_format_time(context, cred->times.starttime, t1, sizeof(t1), 1);
    else
	krb5_format_time(context, cred->times.authtime, t1, sizeof(t1), 1);

    if(cred->times.endtime > sec)
	krb5_format_time(context, cred->times.endtime, t2, sizeof(t2), 1);
    else
	strlcpy(t2, ">>>Expired<<<", sizeof(t2));

    ret = krb5_unparse_name (context, cred->server, &str);
    if (ret) {
	lreply(500, "krb5_unparse_name: %d", ret);
	return 1;
    }

    lreply(200, "%-20s %-20s %s", t1, t2, str);
    free(str);
    return 0;
}

static int
print_tickets (krb5_context context,
	       krb5_ccache ccache,
	       krb5_principal principal)
{
    krb5_error_code ret;
    krb5_cc_cursor cursor;
    krb5_creds cred;
    char *str;

    ret = krb5_unparse_name (context, principal, &str);
    if (ret) {
	lreply(500, "krb5_unparse_name: %d", ret);
	return 500;
    }

    lreply(200, "%17s: %s:%s",
	   "Credentials cache",
	   krb5_cc_get_type(context, ccache),
	   krb5_cc_get_name(context, ccache));
    lreply(200, "%17s: %s", "Principal", str);
    free (str);

    ret = krb5_cc_start_seq_get (context, ccache, &cursor);
    if (ret) {
	lreply(500, "krb5_cc_start_seq_get: %d", ret);
	return 500;
    }

    lreply(200, "  Issued               Expires              Principal");

    while ((ret = krb5_cc_next_cred (context,
				     ccache,
				     &cursor,
				     &cred)) == 0) {
	if (print_cred(context, &cred))
	    return 500;
	krb5_free_cred_contents (context, &cred);
    }
    if (ret != KRB5_CC_END) {
	lreply(500, "krb5_cc_get_next: %d", ret);
	return 500;
    }
    ret = krb5_cc_end_seq_get (context, ccache, &cursor);
    if (ret) {
	lreply(500, "krb5_cc_end_seq_get: %d", ret);
	return 500;
    }

    return 200;
}

static int
klist5(void)
{
    krb5_error_code ret;
    krb5_context context;
    krb5_ccache ccache;
    krb5_principal principal;
    int exit_status = 200;

    ret = krb5_init_context (&context);
    if (ret) {
	lreply(500, "krb5_init_context failed: %d", ret);
	return 500;
    }

    if (k5ccname)
	ret = krb5_cc_resolve(context, k5ccname, &ccache);
    else
	ret = krb5_cc_default (context, &ccache);
    if (ret) {
	lreply(500, "krb5_cc_default: %d", ret);
	return 500;
    }

    ret = krb5_cc_get_principal (context, ccache, &principal);
    if (ret) {
	if(ret == ENOENT)
	    lreply(500, "No ticket file: %s",
		   krb5_cc_get_name(context, ccache));
	else
	    lreply(500, "krb5_cc_get_principal: %d", ret);

	return 500;
    }
    exit_status = print_tickets (context, ccache, principal);

    ret = krb5_cc_close (context, ccache);
    if (ret) {
	lreply(500, "krb5_cc_close: %d", ret);
	exit_status = 500;
    }

    krb5_free_principal (context, principal);
    krb5_free_context (context);
    return exit_status;
}
#endif

void
klist(void)
{
#if KRB5
    int res = klist5();
    reply(res, " ");
#else
    reply(500, "Command not implemented.");
#endif
}

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

/*
 * Try to find out what's a reasonable default principal.
 */

static const char*
get_env_user(void)
{
    const char *user = getenv("USER");
    if(user == NULL)
	user = getenv("LOGNAME");
    if(user == NULL)
	user = getenv("USERNAME");
    return user;
}

#ifndef _WIN32

/*
 * Will only use operating-system dependant operation to get the
 * default principal, for use of functions that in ccache layer to
 * avoid recursive calls.
 */

krb5_error_code
_krb5_get_default_principal_local (krb5_context context,
				   krb5_principal *princ)
{
    krb5_error_code ret;
    const char *user;
    uid_t uid;

    *princ = NULL;

    uid = getuid();
    if(uid == 0) {
	user = getlogin();
	if(user == NULL)
	    user = get_env_user();
	if(user != NULL && strcmp(user, "root") != 0)
	    ret = krb5_make_principal(context, princ, NULL, user, "root", NULL);
	else
	    ret = krb5_make_principal(context, princ, NULL, "root", NULL);
    } else {
	struct passwd *pw = getpwuid(uid);
	if(pw != NULL)
	    user = pw->pw_name;
	else {
	    user = get_env_user();
	    if(user == NULL)
		user = getlogin();
	}
	if(user == NULL) {
	    krb5_set_error_message(context, ENOTTY,
				   N_("unable to figure out current "
				      "principal", ""));
	    return ENOTTY; /* XXX */
	}
	ret = krb5_make_principal(context, princ, NULL, user, NULL);
    }
    return ret;
}

#else  /* _WIN32 */

#define SECURITY_WIN32
#include <security.h>

krb5_error_code
_krb5_get_default_principal_local(krb5_context context,
				  krb5_principal *princ)
{
    /* See if we can get the principal first.  We only expect this to
       work if logged into a domain. */
    {
	char username[1024];
	ULONG sz = sizeof(username);

	if (GetUserNameEx(NameUserPrincipal, username, &sz)) {
	    return krb5_parse_name_flags(context, username,
					 KRB5_PRINCIPAL_PARSE_ENTERPRISE,
					 princ);
	}
    }

    /* Just get the Windows username.  This should pretty much always
       work. */
    {
	char username[1024];
	DWORD dsz = sizeof(username);

	if (GetUserName(username, &dsz)) {
	    return krb5_make_principal(context, princ, NULL, username, NULL);
	}
    }

    /* Failing that, we look at the environment */
    {
	const char * username = get_env_user();

	if (username == NULL) {
	    krb5_set_error_string(context,
				  "unable to figure out current principal");
	    return ENOTTY;	/* Really? */
	}

	return krb5_make_principal(context, princ, NULL, username, NULL);
    }
}

#endif

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_get_default_principal (krb5_context context,
			    krb5_principal *princ)
{
    krb5_error_code ret;
    krb5_ccache id;

    *princ = NULL;

    ret = krb5_cc_default (context, &id);
    if (ret == 0) {
	ret = krb5_cc_get_principal (context, id, princ);
	krb5_cc_close (context, id);
	if (ret == 0)
	    return 0;
    }

    return _krb5_get_default_principal_local(context, princ);
}
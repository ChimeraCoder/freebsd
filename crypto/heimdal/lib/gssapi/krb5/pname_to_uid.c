
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

OM_uint32 GSSAPI_CALLCONV
_gsskrb5_pname_to_uid(OM_uint32 *minor_status,
                      const gss_name_t pname,
                      const gss_OID mech_type,
                      uid_t *uidp)
{
#ifdef NO_LOCALNAME
    *minor_status = KRB5_NO_LOCALNAME;
    return GSS_S_FAILURE;
#else
    krb5_error_code ret;
    krb5_context context;
    krb5_const_principal princ = (krb5_const_principal)pname;
    char localname[256];
#ifdef POSIX_GETPWNAM_R
    char pwbuf[2048];
    struct passwd pw, *pwd;
#else
    struct passwd *pwd;
#endif

    GSSAPI_KRB5_INIT(&context);

    *minor_status = 0;

    ret = krb5_aname_to_localname(context, princ,
                                  sizeof(localname), localname);
    if (ret != 0) {
        *minor_status = ret;
        return GSS_S_FAILURE;
    }

#ifdef POSIX_GETPWNAM_R
    if (getpwnam_r(localname, &pw, pwbuf, sizeof(pwbuf), &pwd) != 0) {
        *minor_status = KRB5_NO_LOCALNAME;
        return GSS_S_FAILURE;
    }
#else
    pwd = getpwnam(localname);
#endif

    if (pwd == NULL) {
        *minor_status = KRB5_NO_LOCALNAME;
        return GSS_S_FAILURE;
    }

    *uidp = pwd->pw_uid;

    return GSS_S_COMPLETE;
#endif /* NO_LOCALNAME */
}

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

#include <pwd.h>

#include "krb5/gsskrb5_locl.h"

OM_uint32
_gsskrb5_pname_to_uid(OM_uint32 *minor_status, const gss_name_t pname,
    const gss_OID mech, uid_t *uidp)
{
	krb5_context context;
	krb5_const_principal name = (krb5_const_principal) pname;
	krb5_error_code kret;
	char lname[MAXLOGNAME + 1], buf[128];
	struct passwd pwd, *pw;

	GSSAPI_KRB5_INIT (&context);

	kret = krb5_aname_to_localname(context, name, sizeof(lname), lname);
	if (kret) {
		*minor_status = kret;
		return (GSS_S_FAILURE);
	}

	*minor_status = 0;
	getpwnam_r(lname, &pwd, buf, sizeof(buf), &pw);
	if (pw) {
		*uidp = pw->pw_uid;
		return (GSS_S_COMPLETE);
	} else {
		return (GSS_S_FAILURE);
	}
}
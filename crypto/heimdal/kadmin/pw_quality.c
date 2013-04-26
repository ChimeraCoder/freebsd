
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

#include "kadmin_locl.h"
#include "kadmin-commands.h"

int
password_quality(void *opt, int argc, char **argv)
{
    krb5_error_code ret;
    krb5_principal principal;
    krb5_data pw_data;
    const char *s;

    ret = krb5_parse_name(context, argv[0], &principal);
    if(ret){
	krb5_warn(context, ret, "krb5_parse_name(%s)", argv[0]);
	return 0;
    }
    pw_data.data = argv[1];
    pw_data.length = strlen(argv[1]);

    s = kadm5_check_password_quality (context, principal, &pw_data);
    if (s)
	krb5_warnx(context, "kadm5_check_password_quality: %s", s);

    krb5_free_principal(context, principal);

    return 0;
}
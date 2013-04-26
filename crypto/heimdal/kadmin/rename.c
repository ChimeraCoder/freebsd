
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
rename_entry(void *opt, int argc, char **argv)
{
    krb5_error_code ret;
    krb5_principal princ1, princ2;

    ret = krb5_parse_name(context, argv[0], &princ1);
    if(ret){
	krb5_warn(context, ret, "krb5_parse_name(%s)", argv[0]);
	return ret != 0;
    }
    ret = krb5_parse_name(context, argv[1], &princ2);
    if(ret){
	krb5_free_principal(context, princ1);
	krb5_warn(context, ret, "krb5_parse_name(%s)", argv[1]);
	return ret != 0;
    }
    ret = kadm5_rename_principal(kadm_handle, princ1, princ2);
    if(ret)
	krb5_warn(context, ret, "rename");
    krb5_free_principal(context, princ1);
    krb5_free_principal(context, princ2);
    return ret != 0;
}
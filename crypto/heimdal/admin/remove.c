
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

#include "ktutil_locl.h"

RCSID("$Id$");

int
kt_remove(struct remove_options *opt, int argc, char **argv)
{
    krb5_error_code ret = 0;
    krb5_keytab_entry entry;
    krb5_keytab keytab;
    krb5_principal principal = NULL;
    krb5_enctype enctype = 0;

    if(opt->principal_string) {
	ret = krb5_parse_name(context, opt->principal_string, &principal);
	if(ret) {
	    krb5_warn(context, ret, "%s", opt->principal_string);
	    return 1;
	}
    }
    if(opt->enctype_string) {
	ret = krb5_string_to_enctype(context, opt->enctype_string, &enctype);
	if(ret) {
	    int t;
	    if(sscanf(opt->enctype_string, "%d", &t) == 1)
		enctype = t;
	    else {
		krb5_warn(context, ret, "%s", opt->enctype_string);
		if(principal)
		    krb5_free_principal(context, principal);
		return 1;
	    }
	}
    }
    if (!principal && !enctype && !opt->kvno_integer) {
	krb5_warnx(context,
		   "You must give at least one of "
		   "principal, enctype or kvno.");
	ret = EINVAL;
	goto out;
    }

    if((keytab = ktutil_open_keytab()) == NULL) {
	ret = 1;
	goto out;
    }

    entry.principal = principal;
    entry.keyblock.keytype = enctype;
    entry.vno = opt->kvno_integer;
    ret = krb5_kt_remove_entry(context, keytab, &entry);
    krb5_kt_close(context, keytab);
    if(ret)
	krb5_warn(context, ret, "remove");
 out:
    if(principal)
	krb5_free_principal(context, principal);
    return ret != 0;
}
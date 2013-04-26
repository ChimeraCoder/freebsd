
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
kt_rename(struct rename_options *opt, int argc, char **argv)
{
    krb5_error_code ret = 0;
    krb5_keytab_entry entry;
    krb5_keytab keytab;
    krb5_kt_cursor cursor;
    krb5_principal from_princ, to_princ;

    ret = krb5_parse_name(context, argv[0], &from_princ);
    if(ret != 0) {
	krb5_warn(context, ret, "%s", argv[0]);
	return 1;
    }

    ret = krb5_parse_name(context, argv[1], &to_princ);
    if(ret != 0) {
	krb5_free_principal(context, from_princ);
	krb5_warn(context, ret, "%s", argv[1]);
	return 1;
    }

    if((keytab = ktutil_open_keytab()) == NULL) {
	krb5_free_principal(context, from_princ);
	krb5_free_principal(context, to_princ);
	return 1;
    }

    ret = krb5_kt_start_seq_get(context, keytab, &cursor);
    if(ret) {
	krb5_kt_close(context, keytab);
	krb5_free_principal(context, from_princ);
	krb5_free_principal(context, to_princ);
	return 1;
    }
    while(1) {
	ret = krb5_kt_next_entry(context, keytab, &entry, &cursor);
	if(ret != 0) {
	    if(ret != KRB5_CC_END && ret != KRB5_KT_END)
		krb5_warn(context, ret, "getting entry from keytab");
	    else
		ret = 0;
	    break;
	}
	if(krb5_principal_compare(context, entry.principal, from_princ)) {
	    krb5_free_principal(context, entry.principal);
	    entry.principal = to_princ;
	    ret = krb5_kt_add_entry(context, keytab, &entry);
	    if(ret) {
		entry.principal = NULL;
		krb5_kt_free_entry(context, &entry);
		krb5_warn(context, ret, "adding entry");
		break;
	    }
	    if (opt->delete_flag) {
		entry.principal = from_princ;
		ret = krb5_kt_remove_entry(context, keytab, &entry);
		if(ret) {
		    entry.principal = NULL;
		    krb5_kt_free_entry(context, &entry);
		    krb5_warn(context, ret, "removing entry");
		    break;
		}
	    }
	    entry.principal = NULL;
	}
	krb5_kt_free_entry(context, &entry);
    }
    krb5_kt_end_seq_get(context, keytab, &cursor);

    krb5_free_principal(context, from_princ);
    krb5_free_principal(context, to_princ);

    return ret != 0;
}
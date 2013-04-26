
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

static char *
readstring(const char *prompt, char *buf, size_t len)
{
    printf("%s", prompt);
    if (fgets(buf, len, stdin) == NULL)
	return NULL;
    buf[strcspn(buf, "\r\n")] = '\0';
    return buf;
}

int
kt_add(struct add_options *opt, int argc, char **argv)
{
    krb5_error_code ret;
    krb5_keytab keytab;
    krb5_keytab_entry entry;
    char buf[1024];
    krb5_enctype enctype;

    if((keytab = ktutil_open_keytab()) == NULL)
	return 1;

    memset(&entry, 0, sizeof(entry));
    if(opt->principal_string == NULL) {
	if(readstring("Principal: ", buf, sizeof(buf)) == NULL)
	    return 1;
	opt->principal_string = buf;
    }
    ret = krb5_parse_name(context, opt->principal_string, &entry.principal);
    if(ret) {
	krb5_warn(context, ret, "%s", opt->principal_string);
	goto out;
    }
    if(opt->enctype_string == NULL) {
	if(readstring("Encryption type: ", buf, sizeof(buf)) == NULL) {
	    ret = 1;
	    goto out;
	}
	opt->enctype_string = buf;
    }
    ret = krb5_string_to_enctype(context, opt->enctype_string, &enctype);
    if(ret) {
	int t;
	if(sscanf(opt->enctype_string, "%d", &t) == 1)
	    enctype = t;
	else {
	    krb5_warn(context, ret, "%s", opt->enctype_string);
	    goto out;
	}
    }
    if(opt->kvno_integer == -1) {
	if(readstring("Key version: ", buf, sizeof(buf)) == NULL) {
	    ret = 1;
	    goto out;
	}
	if(sscanf(buf, "%u", &opt->kvno_integer) != 1)
	    goto out;
    }
    if(opt->password_string == NULL && opt->random_flag == 0) {
	if(UI_UTIL_read_pw_string(buf, sizeof(buf), "Password: ", 1)) {
	    ret = 1;
	    goto out;
	}
	opt->password_string = buf;
    }
    if(opt->password_string) {
	if (opt->hex_flag) {
	    size_t len;
	    void *data;

	    len = (strlen(opt->password_string) + 1) / 2;

	    data = malloc(len);
	    if (data == NULL) {
		krb5_warn(context, ENOMEM, "malloc");
		goto out;
	    }

	    if ((size_t)hex_decode(opt->password_string, data, len) != len) {
		free(data);
		krb5_warn(context, ENOMEM, "hex decode failed");
		goto out;
	    }

	    ret = krb5_keyblock_init(context, enctype,
				     data, len, &entry.keyblock);
	    free(data);
	} else if (!opt->salt_flag) {
	    krb5_salt salt;
	    krb5_data pw;

	    salt.salttype         = KRB5_PW_SALT;
	    salt.saltvalue.data   = NULL;
	    salt.saltvalue.length = 0;
	    pw.data = (void*)opt->password_string;
	    pw.length = strlen(opt->password_string);
	    ret = krb5_string_to_key_data_salt(context, enctype, pw, salt,
					       &entry.keyblock);
        } else {
	    ret = krb5_string_to_key(context, enctype, opt->password_string,
				     entry.principal, &entry.keyblock);
	}
	memset (opt->password_string, 0, strlen(opt->password_string));
    } else {
	ret = krb5_generate_random_keyblock(context, enctype, &entry.keyblock);
    }
    if(ret) {
	krb5_warn(context, ret, "add");
	goto out;
    }
    entry.vno = opt->kvno_integer;
    entry.timestamp = time (NULL);
    ret = krb5_kt_add_entry(context, keytab, &entry);
    if(ret)
	krb5_warn(context, ret, "add");
 out:
    krb5_kt_free_entry(context, &entry);
    krb5_kt_close(context, keytab);
    return ret != 0;
}
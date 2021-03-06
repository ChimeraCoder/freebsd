
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

struct cpw_entry_data {
    int random_key;
    int random_password;
    char *password;
    krb5_key_data *key_data;
};

static int
set_random_key (krb5_principal principal)
{
    krb5_error_code ret;
    int i;
    krb5_keyblock *keys;
    int num_keys;

    ret = kadm5_randkey_principal(kadm_handle, principal, &keys, &num_keys);
    if(ret)
	return ret;
    for(i = 0; i < num_keys; i++)
	krb5_free_keyblock_contents(context, &keys[i]);
    free(keys);
    return 0;
}

static int
set_random_password (krb5_principal principal)
{
    krb5_error_code ret;
    char pw[128];

    random_password (pw, sizeof(pw));
    ret = kadm5_chpass_principal(kadm_handle, principal, pw);
    if (ret == 0) {
	char *princ_name;

	krb5_unparse_name(context, principal, &princ_name);

	printf ("%s's password set to \"%s\"\n", princ_name, pw);
	free (princ_name);
    }
    memset (pw, 0, sizeof(pw));
    return ret;
}

static int
set_password (krb5_principal principal, char *password)
{
    krb5_error_code ret = 0;
    char pwbuf[128];

    if(password == NULL) {
	char *princ_name;
	char *prompt;

	krb5_unparse_name(context, principal, &princ_name);
	asprintf(&prompt, "%s's Password: ", princ_name);
	free (princ_name);
	ret = UI_UTIL_read_pw_string(pwbuf, sizeof(pwbuf), prompt, 1);
	free (prompt);
	if(ret){
	    return 0; /* XXX error code? */
	}
	password = pwbuf;
    }
    if(ret == 0)
	ret = kadm5_chpass_principal(kadm_handle, principal, password);
    memset(pwbuf, 0, sizeof(pwbuf));
    return ret;
}

static int
set_key_data (krb5_principal principal, krb5_key_data *key_data)
{
    krb5_error_code ret;

    ret = kadm5_chpass_principal_with_key (kadm_handle, principal,
					   3, key_data);
    return ret;
}

static int
do_cpw_entry(krb5_principal principal, void *data)
{
    struct cpw_entry_data *e = data;

    if (e->random_key)
	return set_random_key (principal);
    else if (e->random_password)
	return set_random_password (principal);
    else if (e->key_data)
	return set_key_data (principal, e->key_data);
    else
	return set_password (principal, e->password);
}

int
cpw_entry(struct passwd_options *opt, int argc, char **argv)
{
    krb5_error_code ret = 0;
    int i;
    struct cpw_entry_data data;
    int num;
    krb5_key_data key_data[3];

    data.random_key = opt->random_key_flag;
    data.random_password = opt->random_password_flag;
    data.password = opt->password_string;
    data.key_data	 = NULL;

    num = 0;
    if (data.random_key)
	++num;
    if (data.random_password)
	++num;
    if (data.password)
	++num;
    if (opt->key_string)
	++num;

    if (num > 1) {
	fprintf (stderr, "give only one of "
		"--random-key, --random-password, --password, --key\n");
	return 1;
    }

    if (opt->key_string) {
	const char *error;

	if (parse_des_key (opt->key_string, key_data, &error)) {
	    fprintf (stderr, "failed parsing key \"%s\": %s\n",
		     opt->key_string, error);
	    return 1;
	}
	data.key_data = key_data;
    }

    for(i = 0; i < argc; i++)
	ret = foreach_principal(argv[i], do_cpw_entry, "cpw", &data);

    if (data.key_data) {
	int16_t dummy;
	kadm5_free_key_data (kadm_handle, &dummy, key_data);
    }

    return ret != 0;
}
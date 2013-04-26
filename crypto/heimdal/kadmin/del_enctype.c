
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

/*
 * del_enctype principal enctypes...
 */

int
del_enctype(void *opt, int argc, char **argv)
{
    kadm5_principal_ent_rec princ;
    krb5_principal princ_ent = NULL;
    krb5_error_code ret;
    const char *princ_name;
    int i, j, k;
    krb5_key_data *new_key_data;
    int n_etypes;
    krb5_enctype *etypes;

    memset (&princ, 0, sizeof(princ));
    princ_name = argv[0];
    n_etypes   = argc - 1;
    etypes     = malloc (n_etypes * sizeof(*etypes));
    if (etypes == NULL) {
	krb5_warnx (context, "out of memory");
	return 0;
    }
    argv++;
    for (i = 0; i < n_etypes; ++i) {
	ret = krb5_string_to_enctype (context, argv[i], &etypes[i]);
	if (ret) {
	    krb5_warnx (context, "bad enctype \"%s\"", argv[i]);
	    goto out2;
	}
    }

    ret = krb5_parse_name(context, princ_name, &princ_ent);
    if (ret) {
	krb5_warn (context, ret, "krb5_parse_name %s", princ_name);
	goto out2;
    }

    ret = kadm5_get_principal(kadm_handle, princ_ent, &princ,
			      KADM5_PRINCIPAL | KADM5_KEY_DATA);
    if (ret) {
	krb5_free_principal (context, princ_ent);
	krb5_warnx (context, "no such principal: %s", princ_name);
	goto out2;
    }

    new_key_data   = malloc(princ.n_key_data * sizeof(*new_key_data));
    if (new_key_data == NULL && princ.n_key_data != 0) {
	krb5_warnx (context, "out of memory");
	goto out;
    }

    for (i = 0, j = 0; i < princ.n_key_data; ++i) {
	krb5_key_data *key = &princ.key_data[i];
	int docopy = 1;

	for (k = 0; k < n_etypes; ++k)
	    if (etypes[k] == key->key_data_type[0]) {
		docopy = 0;
		break;
	    }
	if (docopy) {
	    new_key_data[j++] = *key;
	} else {
	    int16_t ignore = 1;

	    kadm5_free_key_data (kadm_handle, &ignore, key);
	}
    }

    free (princ.key_data);
    princ.n_key_data = j;
    princ.key_data   = new_key_data;

    ret = kadm5_modify_principal (kadm_handle, &princ, KADM5_KEY_DATA);
    if (ret)
	krb5_warn(context, ret, "kadm5_modify_principal");
out:
    krb5_free_principal (context, princ_ent);
    kadm5_free_principal_ent(kadm_handle, &princ);
out2:
    free (etypes);
    return ret != 0;
}
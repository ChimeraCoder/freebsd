
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

struct ext_keytab_data {
    krb5_keytab keytab;
};

static int
do_ext_keytab(krb5_principal principal, void *data)
{
    krb5_error_code ret;
    kadm5_principal_ent_rec princ;
    struct ext_keytab_data *e = data;
    krb5_keytab_entry *keys = NULL;
    krb5_keyblock *k = NULL;
    int i, n_k;

    ret = kadm5_get_principal(kadm_handle, principal, &princ,
			      KADM5_PRINCIPAL|KADM5_KVNO|KADM5_KEY_DATA);
    if(ret)
	return ret;

    if (princ.n_key_data) {
	keys = malloc(sizeof(*keys) * princ.n_key_data);
	if (keys == NULL) {
	    kadm5_free_principal_ent(kadm_handle, &princ);
	    krb5_clear_error_message(context);
	    return ENOMEM;
	}
	for (i = 0; i < princ.n_key_data; i++) {
	    krb5_key_data *kd = &princ.key_data[i];

	    keys[i].principal = princ.principal;
	    keys[i].vno = kd->key_data_kvno;
	    keys[i].keyblock.keytype = kd->key_data_type[0];
	    keys[i].keyblock.keyvalue.length = kd->key_data_length[0];
	    keys[i].keyblock.keyvalue.data = kd->key_data_contents[0];
	    keys[i].timestamp = time(NULL);
	}

	n_k = princ.n_key_data;
    } else {
	ret = kadm5_randkey_principal(kadm_handle, principal, &k, &n_k);
	if (ret) {
	    kadm5_free_principal_ent(kadm_handle, &princ);
	    return ret;
	}
	keys = malloc(sizeof(*keys) * n_k);
	if (keys == NULL) {
	    kadm5_free_principal_ent(kadm_handle, &princ);
	    krb5_clear_error_message(context);
	    return ENOMEM;
	}
	for (i = 0; i < n_k; i++) {
	    keys[i].principal = principal;
	    keys[i].vno = princ.kvno + 1; /* XXX get entry again */
	    keys[i].keyblock = k[i];
	    keys[i].timestamp = time(NULL);
	}
    }

    for(i = 0; i < n_k; i++) {
	ret = krb5_kt_add_entry(context, e->keytab, &keys[i]);
	if(ret)
	    krb5_warn(context, ret, "krb5_kt_add_entry(%d)", i);
    }

    if (k) {
	memset(k, 0, n_k * sizeof(*k));
	free(k);
    }
    if (keys)
	free(keys);
    kadm5_free_principal_ent(kadm_handle, &princ);
    return 0;
}

int
ext_keytab(struct ext_keytab_options *opt, int argc, char **argv)
{
    krb5_error_code ret;
    int i;
    struct ext_keytab_data data;

    if (opt->keytab_string == NULL)
	ret = krb5_kt_default(context, &data.keytab);
    else
	ret = krb5_kt_resolve(context, opt->keytab_string, &data.keytab);

    if(ret){
	krb5_warn(context, ret, "krb5_kt_resolve");
	return 1;
    }

    for(i = 0; i < argc; i++) {
	ret = foreach_principal(argv[i], do_ext_keytab, "ext", &data);
	if (ret)
	    break;
    }

    krb5_kt_close(context, data.keytab);

    return ret != 0;
}
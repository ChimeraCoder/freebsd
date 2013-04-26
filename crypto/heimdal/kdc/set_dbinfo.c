
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

#include "kdc_locl.h"

static krb5_error_code
add_db(krb5_context context, struct krb5_kdc_configuration *c,
       const char *conf, const char *master_key)
{
    krb5_error_code ret;
    void *ptr;

    ptr = realloc(c->db, (c->num_db + 1) * sizeof(*c->db));
    if (ptr == NULL) {
	krb5_set_error_message(context, ENOMEM, "malloc: out of memory");
	return ENOMEM;
    }
    c->db = ptr;

    ret = hdb_create(context, &c->db[c->num_db], conf);
    if(ret)
	return ret;

    c->num_db++;

    if (master_key) {
	ret = hdb_set_master_keyfile(context, c->db[c->num_db - 1], master_key);
	if (ret)
	    return ret;
    }

    return 0;
}

krb5_error_code
krb5_kdc_set_dbinfo(krb5_context context, struct krb5_kdc_configuration *c)
{
    struct hdb_dbinfo *info, *d;
    krb5_error_code ret;
    int i;

    /* fetch the databases */
    ret = hdb_get_dbinfo(context, &info);
    if (ret)
	return ret;

    d = NULL;
    while ((d = hdb_dbinfo_get_next(info, d)) != NULL) {

	ret = add_db(context, c,
		     hdb_dbinfo_get_dbname(context, d),
		     hdb_dbinfo_get_mkey_file(context, d));
	if (ret)
	    goto out;

	kdc_log(context, c, 0, "label: %s",
		hdb_dbinfo_get_label(context, d));
	kdc_log(context, c, 0, "\tdbname: %s",
		hdb_dbinfo_get_dbname(context, d));
	kdc_log(context, c, 0, "\tmkey_file: %s",
		hdb_dbinfo_get_mkey_file(context, d));
	kdc_log(context, c, 0, "\tacl_file: %s",
		hdb_dbinfo_get_acl_file(context, d));
    }
    hdb_free_dbinfo(context, &info);

    return 0;
out:
    for (i = 0; i < c->num_db; i++)
	if (c->db[i] && c->db[i]->hdb_destroy)
	    (*c->db[i]->hdb_destroy)(context, c->db[i]);
    c->num_db = 0;
    free(c->db);
    c->db = NULL;

    hdb_free_dbinfo(context, &info);

    return ret;
}

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

#include "kadm5_locl.h"

RCSID("$Id$");

struct foreach_data {
    const char *exp;
    char *exp2;
    char **princs;
    int count;
};

static krb5_error_code
add_princ(struct foreach_data *d, char *princ)
{
    char **tmp;
    tmp = realloc(d->princs, (d->count + 1) * sizeof(*tmp));
    if(tmp == NULL)
	return ENOMEM;
    d->princs = tmp;
    d->princs[d->count++] = princ;
    return 0;
}

static krb5_error_code
foreach(krb5_context context, HDB *db, hdb_entry_ex *ent, void *data)
{
    struct foreach_data *d = data;
    char *princ;
    krb5_error_code ret;
    ret = krb5_unparse_name(context, ent->entry.principal, &princ);
    if(ret)
	return ret;
    if(d->exp){
	if(fnmatch(d->exp, princ, 0) == 0 || fnmatch(d->exp2, princ, 0) == 0)
	    ret = add_princ(d, princ);
	else
	    free(princ);
    }else{
	ret = add_princ(d, princ);
    }
    if(ret)
	free(princ);
    return ret;
}

kadm5_ret_t
kadm5_s_get_principals(void *server_handle,
		       const char *expression,
		       char ***princs,
		       int *count)
{
    struct foreach_data d;
    kadm5_server_context *context = server_handle;
    kadm5_ret_t ret;
    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret) {
	krb5_warn(context->context, ret, "opening database");
	return ret;
    }
    d.exp = expression;
    {
	krb5_realm r;
	krb5_get_default_realm(context->context, &r);
	asprintf(&d.exp2, "%s@%s", expression, r);
	free(r);
    }
    d.princs = NULL;
    d.count = 0;
    ret = hdb_foreach(context->context, context->db, HDB_F_ADMIN_DATA, foreach, &d);
    context->db->hdb_close(context->context, context->db);
    if(ret == 0)
	ret = add_princ(&d, NULL);
    if(ret == 0){
	*princs = d.princs;
	*count = d.count - 1;
    }else
	kadm5_free_name_list(context, d.princs, &d.count);
    free(d.exp2);
    return _kadm5_error_code(ret);
}
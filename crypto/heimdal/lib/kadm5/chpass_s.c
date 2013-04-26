
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

static kadm5_ret_t
change(void *server_handle,
       krb5_principal princ,
       const char *password,
       int cond)
{
    kadm5_server_context *context = server_handle;
    hdb_entry_ex ent;
    kadm5_ret_t ret;
    Key *keys;
    size_t num_keys;
    int existsp = 0;

    memset(&ent, 0, sizeof(ent));
    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret)
	return ret;

    ret = context->db->hdb_fetch_kvno(context->context, context->db, princ,
				      HDB_F_DECRYPT|HDB_F_GET_ANY|HDB_F_ADMIN_DATA, 0, &ent);
    if(ret)
	goto out;

    if (context->db->hdb_capability_flags & HDB_CAP_F_HANDLE_PASSWORDS) {
	ret = context->db->hdb_password(context->context, context->db,
					&ent, password, cond);
	if (ret)
	    goto out2;
    } else {

	num_keys = ent.entry.keys.len;
	keys     = ent.entry.keys.val;

	ent.entry.keys.len = 0;
	ent.entry.keys.val = NULL;

	ret = _kadm5_set_keys(context, &ent.entry, password);
	if(ret) {
	    _kadm5_free_keys (context->context, num_keys, keys);
	    goto out2;
	}

	if (cond)
	    existsp = _kadm5_exists_keys (ent.entry.keys.val,
					  ent.entry.keys.len,
					  keys, num_keys);
	_kadm5_free_keys (context->context, num_keys, keys);

	if (existsp) {
	    ret = KADM5_PASS_REUSE;
	    krb5_set_error_message(context->context, ret,
				   "Password reuse forbidden");
	    goto out2;
	}

	ret = hdb_seal_keys(context->context, context->db, &ent.entry);
	if (ret)
	    goto out2;
    }
    ent.entry.kvno++;

    ret = _kadm5_set_modifier(context, &ent.entry);
    if(ret)
	goto out2;

    ret = _kadm5_bump_pw_expire(context, &ent.entry);
    if (ret)
	goto out2;

    ret = context->db->hdb_store(context->context, context->db,
				 HDB_F_REPLACE, &ent);
    if (ret)
	goto out2;

    kadm5_log_modify (context,
		      &ent.entry,
		      KADM5_PRINCIPAL | KADM5_MOD_NAME | KADM5_MOD_TIME |
		      KADM5_KEY_DATA | KADM5_KVNO | KADM5_PW_EXPIRATION |
		      KADM5_TL_DATA);

out2:
    hdb_free_entry(context->context, &ent);
out:
    context->db->hdb_close(context->context, context->db);
    return _kadm5_error_code(ret);
}



/*
 * change the password of `princ' to `password' if it's not already that.
 */

kadm5_ret_t
kadm5_s_chpass_principal_cond(void *server_handle,
			      krb5_principal princ,
			      const char *password)
{
    return change (server_handle, princ, password, 1);
}

/*
 * change the password of `princ' to `password'
 */

kadm5_ret_t
kadm5_s_chpass_principal(void *server_handle,
			 krb5_principal princ,
			 const char *password)
{
    return change (server_handle, princ, password, 0);
}

/*
 * change keys for `princ' to `keys'
 */

kadm5_ret_t
kadm5_s_chpass_principal_with_key(void *server_handle,
				  krb5_principal princ,
				  int n_key_data,
				  krb5_key_data *key_data)
{
    kadm5_server_context *context = server_handle;
    hdb_entry_ex ent;
    kadm5_ret_t ret;

    memset(&ent, 0, sizeof(ent));
    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret)
	return ret;
    ret = context->db->hdb_fetch_kvno(context->context, context->db, princ, 0,
				      HDB_F_GET_ANY|HDB_F_ADMIN_DATA, &ent);
    if(ret == HDB_ERR_NOENTRY)
	goto out;
    ret = _kadm5_set_keys2(context, &ent.entry, n_key_data, key_data);
    if(ret)
	goto out2;
    ent.entry.kvno++;
    ret = _kadm5_set_modifier(context, &ent.entry);
    if(ret)
	goto out2;
    ret = _kadm5_bump_pw_expire(context, &ent.entry);
    if (ret)
	goto out2;

    ret = hdb_seal_keys(context->context, context->db, &ent.entry);
    if (ret)
	goto out2;

    ret = context->db->hdb_store(context->context, context->db,
				 HDB_F_REPLACE, &ent);
    if (ret)
	goto out2;

    kadm5_log_modify (context,
		      &ent.entry,
		      KADM5_PRINCIPAL | KADM5_MOD_NAME | KADM5_MOD_TIME |
		      KADM5_KEY_DATA | KADM5_KVNO | KADM5_PW_EXPIRATION |
		      KADM5_TL_DATA);

out2:
    hdb_free_entry(context->context, &ent);
out:
    context->db->hdb_close(context->context, context->db);
    return _kadm5_error_code(ret);
}
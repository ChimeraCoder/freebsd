
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

kadm5_ret_t
kadm5_s_rename_principal(void *server_handle,
			 krb5_principal source,
			 krb5_principal target)
{
    kadm5_server_context *context = server_handle;
    kadm5_ret_t ret;
    hdb_entry_ex ent;
    krb5_principal oldname;

    memset(&ent, 0, sizeof(ent));
    if(krb5_principal_compare(context->context, source, target))
	return KADM5_DUP; /* XXX is this right? */
    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret)
	return ret;
    ret = context->db->hdb_fetch_kvno(context->context, context->db,
				      source, HDB_F_GET_ANY|HDB_F_ADMIN_DATA, 0, &ent);
    if(ret){
	context->db->hdb_close(context->context, context->db);
	goto out;
    }
    ret = _kadm5_set_modifier(context, &ent.entry);
    if(ret)
	goto out2;
    {
	/* fix salt */
	size_t i;
	Salt salt;
	krb5_salt salt2;
	memset(&salt, 0, sizeof(salt));
	krb5_get_pw_salt(context->context, source, &salt2);
	salt.type = hdb_pw_salt;
	salt.salt = salt2.saltvalue;
	for(i = 0; i < ent.entry.keys.len; i++){
	    if(ent.entry.keys.val[i].salt == NULL){
		ent.entry.keys.val[i].salt =
		    malloc(sizeof(*ent.entry.keys.val[i].salt));
		if(ent.entry.keys.val[i].salt == NULL)
		    return ENOMEM;
		ret = copy_Salt(&salt, ent.entry.keys.val[i].salt);
		if(ret)
		    break;
	    }
	}
	krb5_free_salt(context->context, salt2);
    }
    if(ret)
	goto out2;
    oldname = ent.entry.principal;
    ent.entry.principal = target;

    ret = hdb_seal_keys(context->context, context->db, &ent.entry);
    if (ret) {
	ent.entry.principal = oldname;
	goto out2;
    }

    kadm5_log_rename (context, source, &ent.entry);

    ret = context->db->hdb_store(context->context, context->db, 0, &ent);
    if(ret){
	ent.entry.principal = oldname;
	goto out2;
    }
    ret = context->db->hdb_remove(context->context, context->db, oldname);
    ent.entry.principal = oldname;
out2:
    context->db->hdb_close(context->context, context->db);
    hdb_free_entry(context->context, &ent);
out:
    return _kadm5_error_code(ret);
}
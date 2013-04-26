
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
modify_principal(void *server_handle,
		 kadm5_principal_ent_t princ,
		 uint32_t mask,
		 uint32_t forbidden_mask)
{
    kadm5_server_context *context = server_handle;
    hdb_entry_ex ent;
    kadm5_ret_t ret;
    if((mask & forbidden_mask))
	return KADM5_BAD_MASK;
    if((mask & KADM5_POLICY) && strcmp(princ->policy, "default"))
	return KADM5_UNK_POLICY;

    memset(&ent, 0, sizeof(ent));
    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret)
	return ret;
    ret = context->db->hdb_fetch_kvno(context->context, context->db,
				      princ->principal, HDB_F_GET_ANY|HDB_F_ADMIN_DATA, 0, &ent);
    if(ret)
	goto out;
    ret = _kadm5_setup_entry(context, &ent, mask, princ, mask, NULL, 0);
    if(ret)
	goto out2;
    ret = _kadm5_set_modifier(context, &ent.entry);
    if(ret)
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
		      mask | KADM5_MOD_NAME | KADM5_MOD_TIME);

out2:
    hdb_free_entry(context->context, &ent);
out:
    context->db->hdb_close(context->context, context->db);
    return _kadm5_error_code(ret);
}


kadm5_ret_t
kadm5_s_modify_principal(void *server_handle,
			 kadm5_principal_ent_t princ,
			 uint32_t mask)
{
    return modify_principal(server_handle, princ, mask,
			    KADM5_LAST_PWD_CHANGE | KADM5_MOD_TIME
			    | KADM5_MOD_NAME | KADM5_MKVNO
			    | KADM5_AUX_ATTRIBUTES | KADM5_LAST_SUCCESS
			    | KADM5_LAST_FAILED);
}
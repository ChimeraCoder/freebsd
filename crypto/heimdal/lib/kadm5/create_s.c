
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
get_default(kadm5_server_context *context, krb5_principal princ,
	    kadm5_principal_ent_t def)
{
    kadm5_ret_t ret;
    krb5_principal def_principal;
    krb5_const_realm realm = krb5_principal_get_realm(context->context, princ);

    ret = krb5_make_principal(context->context, &def_principal,
			      realm, "default", NULL);
    if (ret)
	return ret;
    ret = kadm5_s_get_principal(context, def_principal, def,
				KADM5_PRINCIPAL_NORMAL_MASK);
    krb5_free_principal (context->context, def_principal);
    return ret;
}

static kadm5_ret_t
create_principal(kadm5_server_context *context,
		 kadm5_principal_ent_t princ,
		 uint32_t mask,
		 hdb_entry_ex *ent,
		 uint32_t required_mask,
		 uint32_t forbidden_mask)
{
    kadm5_ret_t ret;
    kadm5_principal_ent_rec defrec, *defent;
    uint32_t def_mask;

    if((mask & required_mask) != required_mask)
	return KADM5_BAD_MASK;
    if((mask & forbidden_mask))
	return KADM5_BAD_MASK;
    if((mask & KADM5_POLICY) && strcmp(princ->policy, "default"))
	/* XXX no real policies for now */
	return KADM5_UNK_POLICY;
    memset(ent, 0, sizeof(*ent));
    ret  = krb5_copy_principal(context->context, princ->principal,
			       &ent->entry.principal);
    if(ret)
	return ret;

    defent = &defrec;
    ret = get_default(context, princ->principal, defent);
    if(ret) {
	defent   = NULL;
	def_mask = 0;
    } else {
	def_mask = KADM5_ATTRIBUTES | KADM5_MAX_LIFE | KADM5_MAX_RLIFE;
    }

    ret = _kadm5_setup_entry(context,
			     ent, mask | def_mask,
			     princ, mask,
			     defent, def_mask);
    if(defent)
	kadm5_free_principal_ent(context, defent);
    if (ret)
	return ret;

    ent->entry.created_by.time = time(NULL);

    return krb5_copy_principal(context->context, context->caller,
			       &ent->entry.created_by.principal);
}

kadm5_ret_t
kadm5_s_create_principal_with_key(void *server_handle,
				  kadm5_principal_ent_t princ,
				  uint32_t mask)
{
    kadm5_ret_t ret;
    hdb_entry_ex ent;
    kadm5_server_context *context = server_handle;

    ret = create_principal(context, princ, mask, &ent,
			   KADM5_PRINCIPAL | KADM5_KEY_DATA,
			   KADM5_LAST_PWD_CHANGE | KADM5_MOD_TIME
			   | KADM5_MOD_NAME | KADM5_MKVNO
			   | KADM5_AUX_ATTRIBUTES
			   | KADM5_POLICY_CLR | KADM5_LAST_SUCCESS
			   | KADM5_LAST_FAILED | KADM5_FAIL_AUTH_COUNT);
    if(ret)
	goto out;

    if ((mask & KADM5_KVNO) == 0)
	ent.entry.kvno = 1;

    ret = hdb_seal_keys(context->context, context->db, &ent.entry);
    if (ret)
	goto out;

    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret)
	goto out;
    ret = context->db->hdb_store(context->context, context->db, 0, &ent);
    context->db->hdb_close(context->context, context->db);
    if (ret)
	goto out;
    kadm5_log_create (context, &ent.entry);

out:
    hdb_free_entry(context->context, &ent);
    return _kadm5_error_code(ret);
}


kadm5_ret_t
kadm5_s_create_principal(void *server_handle,
			 kadm5_principal_ent_t princ,
			 uint32_t mask,
			 const char *password)
{
    kadm5_ret_t ret;
    hdb_entry_ex ent;
    kadm5_server_context *context = server_handle;

    ret = create_principal(context, princ, mask, &ent,
			   KADM5_PRINCIPAL,
			   KADM5_LAST_PWD_CHANGE | KADM5_MOD_TIME
			   | KADM5_MOD_NAME | KADM5_MKVNO
			   | KADM5_AUX_ATTRIBUTES | KADM5_KEY_DATA
			   | KADM5_POLICY_CLR | KADM5_LAST_SUCCESS
			   | KADM5_LAST_FAILED | KADM5_FAIL_AUTH_COUNT);
    if(ret)
	goto out;

    if ((mask & KADM5_KVNO) == 0)
	ent.entry.kvno = 1;

    ent.entry.keys.len = 0;
    ent.entry.keys.val = NULL;

    ret = _kadm5_set_keys(context, &ent.entry, password);
    if (ret)
	goto out;

    ret = hdb_seal_keys(context->context, context->db, &ent.entry);
    if (ret)
	goto out;

    ret = context->db->hdb_open(context->context, context->db, O_RDWR, 0);
    if(ret)
	goto out;
    ret = context->db->hdb_store(context->context, context->db, 0, &ent);
    context->db->hdb_close(context->context, context->db);
    if (ret)
	goto out;

    kadm5_log_create (context, &ent.entry);

 out:
    hdb_free_entry(context->context, &ent);
    return _kadm5_error_code(ret);
}
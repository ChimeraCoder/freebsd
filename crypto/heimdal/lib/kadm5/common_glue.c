
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

#define __CALL(F, P) (*((kadm5_common_context*)server_handle)->funcs.F)P;

kadm5_ret_t
kadm5_chpass_principal(void *server_handle,
		       krb5_principal princ,
		       const char *password)
{
    return __CALL(chpass_principal, (server_handle, princ, password));
}

kadm5_ret_t
kadm5_chpass_principal_with_key(void *server_handle,
				krb5_principal princ,
				int n_key_data,
				krb5_key_data *key_data)
{
    return __CALL(chpass_principal_with_key,
		  (server_handle, princ, n_key_data, key_data));
}

kadm5_ret_t
kadm5_create_principal(void *server_handle,
		       kadm5_principal_ent_t princ,
		       uint32_t mask,
		       const char *password)
{
    return __CALL(create_principal, (server_handle, princ, mask, password));
}

kadm5_ret_t
kadm5_delete_principal(void *server_handle,
		       krb5_principal princ)
{
    return __CALL(delete_principal, (server_handle, princ));
}

kadm5_ret_t
kadm5_destroy (void *server_handle)
{
    return __CALL(destroy, (server_handle));
}

kadm5_ret_t
kadm5_flush (void *server_handle)
{
    return __CALL(flush, (server_handle));
}

kadm5_ret_t
kadm5_get_principal(void *server_handle,
		    krb5_principal princ,
		    kadm5_principal_ent_t out,
		    uint32_t mask)
{
    return __CALL(get_principal, (server_handle, princ, out, mask));
}

kadm5_ret_t
kadm5_modify_principal(void *server_handle,
		       kadm5_principal_ent_t princ,
		       uint32_t mask)
{
    return __CALL(modify_principal, (server_handle, princ, mask));
}

kadm5_ret_t
kadm5_randkey_principal(void *server_handle,
			krb5_principal princ,
			krb5_keyblock **new_keys,
			int *n_keys)
{
    return __CALL(randkey_principal, (server_handle, princ, new_keys, n_keys));
}

kadm5_ret_t
kadm5_rename_principal(void *server_handle,
		       krb5_principal source,
		       krb5_principal target)
{
    return __CALL(rename_principal, (server_handle, source, target));
}

kadm5_ret_t
kadm5_get_principals(void *server_handle,
		     const char *expression,
		     char ***princs,
		     int *count)
{
    return __CALL(get_principals, (server_handle, expression, princs, count));
}

kadm5_ret_t
kadm5_get_privs(void *server_handle,
		uint32_t *privs)
{
    return __CALL(get_privs, (server_handle, privs));
}
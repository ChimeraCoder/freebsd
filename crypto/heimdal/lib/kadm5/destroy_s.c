
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

/*
 * dealloc a `kadm5_config_params'
 */

static void
destroy_config (kadm5_config_params *c)
{
    free (c->realm);
    free (c->dbname);
    free (c->acl_file);
    free (c->stash_file);
}

/*
 * dealloc a kadm5_log_context
 */

static void
destroy_kadm5_log_context (kadm5_log_context *c)
{
    free (c->log_file);
    rk_closesocket (c->socket_fd);
#ifdef NO_UNIX_SOCKETS
    if (c->socket_info) {
	freeaddrinfo(c->socket_info);
	c->socket_info = NULL;
    }
#endif
}

/*
 * destroy a kadm5 handle
 */

kadm5_ret_t
kadm5_s_destroy(void *server_handle)
{
    kadm5_ret_t ret;
    kadm5_server_context *context = server_handle;
    krb5_context kcontext = context->context;

    ret = context->db->hdb_destroy(kcontext, context->db);
    destroy_kadm5_log_context (&context->log_context);
    destroy_config (&context->config);
    krb5_free_principal (kcontext, context->caller);
    if(context->my_context)
	krb5_free_context(kcontext);
    free (context);
    return ret;
}
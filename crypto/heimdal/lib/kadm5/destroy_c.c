
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
kadm5_c_destroy(void *server_handle)
{
    kadm5_client_context *context = server_handle;

    free(context->realm);
    free(context->admin_server);
    close(context->sock);
    if (context->client_name)
	free(context->client_name);
    if (context->service_name)
	free(context->service_name);
    if (context->ac != NULL)
	krb5_auth_con_free(context->context, context->ac);
    if(context->my_context)
	krb5_free_context(context->context);
    return 0;
}
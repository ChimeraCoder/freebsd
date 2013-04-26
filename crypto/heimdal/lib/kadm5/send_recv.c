
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
_kadm5_client_send(kadm5_client_context *context, krb5_storage *sp)
{
    krb5_data msg, out;
    krb5_error_code ret;
    size_t len;
    krb5_storage *sock;

    assert(context->sock != -1);

    len = krb5_storage_seek(sp, 0, SEEK_CUR);
    ret = krb5_data_alloc(&msg, len);
    if (ret) {
	krb5_clear_error_message(context->context);
	return ret;
    }
    krb5_storage_seek(sp, 0, SEEK_SET);
    krb5_storage_read(sp, msg.data, msg.length);

    ret = krb5_mk_priv(context->context, context->ac, &msg, &out, NULL);
    krb5_data_free(&msg);
    if(ret)
	return ret;

    sock = krb5_storage_from_fd(context->sock);
    if(sock == NULL) {
	krb5_clear_error_message(context->context);
	krb5_data_free(&out);
	return ENOMEM;
    }

    ret = krb5_store_data(sock, out);
    if (ret)
	krb5_clear_error_message(context->context);
    krb5_storage_free(sock);
    krb5_data_free(&out);
    return ret;
}

kadm5_ret_t
_kadm5_client_recv(kadm5_client_context *context, krb5_data *reply)
{
    krb5_error_code ret;
    krb5_data data;
    krb5_storage *sock;

    sock = krb5_storage_from_fd(context->sock);
    if(sock == NULL) {
	krb5_clear_error_message(context->context);
	return ENOMEM;
    }
    ret = krb5_ret_data(sock, &data);
    krb5_storage_free(sock);
    krb5_clear_error_message(context->context);
    if(ret == KRB5_CC_END)
	return KADM5_RPC_ERROR;
    else if(ret)
	return ret;

    ret = krb5_rd_priv(context->context, context->ac, &data, reply, NULL);
    krb5_data_free(&data);
    return ret;
}
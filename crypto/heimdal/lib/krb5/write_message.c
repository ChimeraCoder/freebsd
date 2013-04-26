
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

#include "krb5_locl.h"

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_write_message (krb5_context context,
		    krb5_pointer p_fd,
		    krb5_data *data)
{
    uint32_t len;
    uint8_t buf[4];
    int ret;

    len = data->length;
    _krb5_put_int(buf, len, 4);
    if (krb5_net_write (context, p_fd, buf, 4) != 4
	|| krb5_net_write (context, p_fd, data->data, len) != len) {
	ret = errno;
	krb5_set_error_message (context, ret, "write: %s", strerror(ret));
	return ret;
    }
    return 0;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_write_priv_message(krb5_context context,
			krb5_auth_context ac,
			krb5_pointer p_fd,
			krb5_data *data)
{
    krb5_error_code ret;
    krb5_data packet;

    ret = krb5_mk_priv (context, ac, data, &packet, NULL);
    if(ret)
	return ret;
    ret = krb5_write_message(context, p_fd, &packet);
    krb5_data_free(&packet);
    return ret;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_write_safe_message(krb5_context context,
			krb5_auth_context ac,
			krb5_pointer p_fd,
			krb5_data *data)
{
    krb5_error_code ret;
    krb5_data packet;
    ret = krb5_mk_safe (context, ac, data, &packet, NULL);
    if(ret)
	return ret;
    ret = krb5_write_message(context, p_fd, &packet);
    krb5_data_free(&packet);
    return ret;
}
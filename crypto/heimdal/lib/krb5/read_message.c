
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
krb5_read_message (krb5_context context,
		   krb5_pointer p_fd,
		   krb5_data *data)
{
    krb5_error_code ret;
    uint32_t len;
    uint8_t buf[4];

    krb5_data_zero(data);

    ret = krb5_net_read (context, p_fd, buf, 4);
    if(ret == -1) {
	ret = errno;
	krb5_clear_error_message (context);
	return ret;
    }
    if(ret < 4) {
	krb5_clear_error_message(context);
	return HEIM_ERR_EOF;
    }
    len = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    ret = krb5_data_alloc (data, len);
    if (ret) {
	krb5_clear_error_message(context);
	return ret;
    }
    if (krb5_net_read (context, p_fd, data->data, len) != len) {
	ret = errno;
	krb5_data_free (data);
	krb5_clear_error_message (context);
	return ret;
    }
    return 0;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_read_priv_message(krb5_context context,
		       krb5_auth_context ac,
		       krb5_pointer p_fd,
		       krb5_data *data)
{
    krb5_error_code ret;
    krb5_data packet;

    ret = krb5_read_message(context, p_fd, &packet);
    if(ret)
	return ret;
    ret = krb5_rd_priv (context, ac, &packet, data, NULL);
    krb5_data_free(&packet);
    return ret;
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_read_safe_message(krb5_context context,
		       krb5_auth_context ac,
		       krb5_pointer p_fd,
		       krb5_data *data)
{
    krb5_error_code ret;
    krb5_data packet;

    ret = krb5_read_message(context, p_fd, &packet);
    if(ret)
	return ret;
    ret = krb5_rd_safe (context, ac, &packet, data, NULL);
    krb5_data_free(&packet);
    return ret;
}
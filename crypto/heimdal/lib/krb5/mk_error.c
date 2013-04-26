
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
krb5_mk_error(krb5_context context,
	      krb5_error_code error_code,
	      const char *e_text,
	      const krb5_data *e_data,
	      const krb5_principal client,
	      const krb5_principal server,
	      time_t *client_time,
	      int *client_usec,
	      krb5_data *reply)
{
    const char *e_text2 = NULL;
    KRB_ERROR msg;
    krb5_timestamp sec;
    int32_t usec;
    size_t len = 0;
    krb5_error_code ret = 0;

    krb5_us_timeofday (context, &sec, &usec);

    memset(&msg, 0, sizeof(msg));
    msg.pvno     = 5;
    msg.msg_type = krb_error;
    msg.stime    = sec;
    msg.susec    = usec;
    msg.ctime    = client_time;
    msg.cusec    = client_usec;
    /* Make sure we only send `protocol' error codes */
    if(error_code < KRB5KDC_ERR_NONE || error_code >= KRB5_ERR_RCSID) {
	if(e_text == NULL)
	    e_text = e_text2 = krb5_get_error_message(context, error_code);
	error_code = KRB5KRB_ERR_GENERIC;
    }
    msg.error_code = error_code - KRB5KDC_ERR_NONE;
    if (e_text)
	msg.e_text = rk_UNCONST(&e_text);
    if (e_data)
	msg.e_data = rk_UNCONST(e_data);
    if(server){
	msg.realm = server->realm;
	msg.sname = server->name;
    }else{
	static char unspec[] = "<unspecified realm>";
	msg.realm = unspec;
    }
    if(client){
	msg.crealm = &client->realm;
	msg.cname = &client->name;
    }

    ASN1_MALLOC_ENCODE(KRB_ERROR, reply->data, reply->length, &msg, &len, ret);
    if (e_text2)
	krb5_free_error_message(context, e_text2);
    if (ret)
	return ret;
    if(reply->length != len)
	krb5_abortx(context, "internal error in ASN.1 encoder");
    return 0;
}

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
krb5_rd_error(krb5_context context,
	      const krb5_data *msg,
	      KRB_ERROR *result)
{

    size_t len;
    krb5_error_code ret;

    ret = decode_KRB_ERROR(msg->data, msg->length, result, &len);
    if(ret) {
	krb5_clear_error_message(context);
	return ret;
    }
    result->error_code += KRB5KDC_ERR_NONE;
    return 0;
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_error_contents (krb5_context context,
			  krb5_error *error)
{
    free_KRB_ERROR(error);
    memset(error, 0, sizeof(*error));
}

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_error (krb5_context context,
		 krb5_error *error)
{
    krb5_free_error_contents (context, error);
    free (error);
}

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_error_from_rd_error(krb5_context context,
			 const krb5_error *error,
			 const krb5_creds *creds)
{
    krb5_error_code ret;

    ret = error->error_code;
    if (error->e_text != NULL) {
	krb5_set_error_message(context, ret, "%s", *error->e_text);
    } else {
	char clientname[256], servername[256];

	if (creds != NULL) {
	    krb5_unparse_name_fixed(context, creds->client,
				    clientname, sizeof(clientname));
	    krb5_unparse_name_fixed(context, creds->server,
				    servername, sizeof(servername));
	}

	switch (ret) {
	case KRB5KDC_ERR_NAME_EXP :
	    krb5_set_error_message(context, ret,
				   N_("Client %s%s%s expired", ""),
				   creds ? "(" : "",
				   creds ? clientname : "",
				   creds ? ")" : "");
	    break;
	case KRB5KDC_ERR_SERVICE_EXP :
	    krb5_set_error_message(context, ret,
				   N_("Server %s%s%s expired", ""),
				   creds ? "(" : "",
				   creds ? servername : "",
				   creds ? ")" : "");
	    break;
	case KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN :
	    krb5_set_error_message(context, ret,
				   N_("Client %s%s%s unknown", ""),
				   creds ? "(" : "",
				   creds ? clientname : "",
				   creds ? ")" : "");
	    break;
	case KRB5KDC_ERR_S_PRINCIPAL_UNKNOWN :
	    krb5_set_error_message(context, ret,
				   N_("Server %s%s%s unknown", ""),
				   creds ? "(" : "",
				   creds ? servername : "",
				   creds ? ")" : "");
	    break;
	default :
	    krb5_clear_error_message(context);
	    break;
	}
    }
    return ret;
}